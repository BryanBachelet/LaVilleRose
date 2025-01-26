// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"

#include "ChaosSQTypes.h"
#include "PaintBrushComponent.h"
#include "PaintingManager.h"
#include "PaintVolume.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define PRINT(x, ...) GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, FString::Printf(L##x, ##__VA_ARGS__));

// Sets default values
ACar::ACar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACar::BeginPlay()
{
	m_CollisionMesh = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName("SM_Collision"));
	m_VehicleMesh = Cast<UPoseableMeshComponent>(GetDefaultSubobjectByName("PM_Vehicle"));
	m_SpringArm = Cast<USpringArmComponent>(GetDefaultSubobjectByName("SpringArm")); 

	if (m_CollisionMesh)
	{
		GroundFriction = m_CollisionMesh->GetLinearDamping();
	}
	
	Super::BeginPlay();
}

void ACar::InitializeWheelLocations()
{
	for (auto WheelBoneName : WheelBoneNames)
	{
		FVector location = m_VehicleMesh->GetBoneLocationByName(WheelBoneName, EBoneSpaces::ComponentSpace);
		WheelInitialLocations.Add(location);
	}
}

void ACar::InitializePainting()
{
	auto paintingManager = Cast<UPaintingManager>(GetWorld()->GetGameState()->GetComponentByClass(UPaintingManager::StaticClass()));

	AActor* actor =  UGameplayStatics::GetActorOfClass(GetWorld(),APaintVolume::StaticClass());
	if(actor == NULL)
	{
		UE_LOG(LogTemp,Error,TEXT(" Dont find volume paint"))
		return;
	}
	if (paintingManager)
	{
		paintingManager->actorWorldScale = actor;
		
		paintingManager->InitializePaintTexture();
	}

	WorldScale = actor->GetActorScale3D();
	WorldOffset =  actor->GetActorLocation();
	
	TArray<UPaintBrushComponent*> paintBrushes;
	paintBrushes.Add(Cast<UPaintBrushComponent>(GetDefaultSubobjectByName("PaintBrushFR")));
	paintBrushes.Add(Cast<UPaintBrushComponent>(GetDefaultSubobjectByName("PaintBrushFL")));

	
	for (auto paintBrushActorComp : paintBrushes)
	{
		auto paintBrush = Cast<UPaintBrushComponent>(paintBrushActorComp);
		paintBrush->InitializeValues(nullptr, WorldScale, WorldOffset);
		paintBrush->BeginRendering();
		
		PaintBrushes.Add(paintBrush);
	}
}

void ACar::TurnVelocityWithRotation(float DeltaTime)
{
	if (!bIsGrounded)
		return;

	FVector linearVelocity = m_CollisionMesh->GetPhysicsLinearVelocity();

	float angle = m_CollisionMesh->GetPhysicsAngularVelocity().Z * DeltaTime;
	FRotator rotator = FRotator::MakeFromEuler(FVector::UpVector * angle);
	linearVelocity = rotator.RotateVector(linearVelocity);
	
	m_CollisionMesh->SetPhysicsLinearVelocity(linearVelocity);
}

void ACar::ComputeSuspension()
{
	FVector location = m_CollisionMesh->GetComponentLocation();
	FVector up = m_CollisionMesh->GetUpVector();

	FHitResult SuspensionTraceHit;
	FVector Start = location;
	FVector End = location - SuspensionLength * up;

	FVector halfSize = m_CollisionMesh->GetComponentScale() * 48.f;
	halfSize.Z = 0.01f;

	FRotator orientation = m_CollisionMesh->GetComponentRotation();

	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), Start, End, halfSize, orientation, TraceTypeQuery1
		, true, {}, EDrawDebugTrace::None, SuspensionTraceHit,  true);

	bIsGrounded = SuspensionTraceHit.bBlockingHit;

	if (bIsGrounded)
	{
		m_CollisionMesh->SetLinearDamping(GroundFriction);
		float force = ((SuspensionLength - SuspensionTraceHit.Distance) / SuspensionLength) * SuspensionForce;
		FVector forceAxis = up;
		m_CollisionMesh->AddForce(forceAxis * force, NAME_None, true);
	}
	else
	{
		m_CollisionMesh->SetLinearDamping(0.1f);
	}
}

void ACar::MoveCamera()
{
	if (!bCameraFollowsPlayer)
		return;
	
	auto location = m_CollisionMesh->GetComponentLocation();
	m_SpringArm->SetWorldLocation(location);
}

void ACar::TiltCar()
{
	float currentSpeed = m_CollisionMesh->GetPhysicsLinearVelocity().Size2D() * 0.01f;
	FVector angularVelocity = m_CollisionMesh->GetPhysicsAngularVelocityInDegrees();
	float desiredTilt = FMath::Clamp(angularVelocity.Z, -150.f, 150.f)
	* DesiredTiltMultiplier
	* currentSpeed
	* ThrustDirection
	* -0.01f;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (DeltaTime > 0.1f)
		return;
	CurrentTilt = UKismetMathLibrary::FloatSpringInterp(CurrentTilt, desiredTilt, TiltSpring, TiltSpringStiffness, TiltSpringDamping, DeltaTime, 1.f);
	FRotator NewRotation(0.f, m_VehicleMesh->GetRelativeRotation().Yaw, CurrentTilt);
	m_VehicleMesh->SetRelativeRotation(NewRotation);
}

void ACar::TurnWheelWhenSteering()
{
	float angle = m_CollisionMesh->GetPhysicsAngularVelocityInDegrees().Z * WheelSteeringAngleMultiplier - 90;
	m_VehicleMesh->SetBoneRotationByName(L"JNT_FrontWheel_L", FRotator(90.f, 0.f, angle), EBoneSpaces::ComponentSpace);
	m_VehicleMesh->SetBoneRotationByName(L"JNT_FrontWheel_R", FRotator(90.f, 0.f, angle), EBoneSpaces::ComponentSpace);
}

void ACar::ProjectWheelsOnGround()
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (DeltaTime > 0.1f)
		return;
	for (size_t i = 0; i < WheelBoneNames.Num(); i++)
	{
		FVector location = UKismetMathLibrary::TransformLocation(m_VehicleMesh->GetComponentTransform(), WheelInitialLocations[i]);
		FVector end = location + FVector::UpVector * -0.5f * SuspensionLength;

		FHitResult hit;
		UKismetSystemLibrary::SphereTraceSingle(GetWorld(), location, end, 30.f, TraceTypeQuery1,
			true, {}, EDrawDebugTrace::None, hit, true);

		OffsetWheelWithSuspension(i, hit.bBlockingHit, hit.Distance);
	}
}

void ACar::ProjectBrushOnGround(UPaintBrushComponent* Brush)
{
	auto location =	Brush->GetAttachParent()->GetComponentLocation();
	FVector end = location + FVector::UpVector * -0.3f * SuspensionLength;

	FHitResult result;
	GetWorld()->LineTraceSingleByChannel(result, location, end, ECollisionChannel::ECC_Visibility);

	FHitResult result2;
	Brush->SetWorldLocation(result.bBlockingHit ? result.ImpactPoint : location, false, &result2, ETeleportType::TeleportPhysics);
}

void ACar::TransitionCamera(float deltaTime)
{
	if(!activeArmLengthTransition) return;

	if(timerArmlength > durationTransitionArmLength)
 	{
		activeArmLengthTransition = false;
		timerArmlength = 0.0f;
	}
	else
	{
		m_SpringArm->TargetArmLength = FMath::Lerp(previousArmLength,targetArmlength,timerArmlength/durationTransitionArmLength);
		timerArmlength += deltaTime;
	}
}

void ACar::ActiveCameraTransitonArmLength(float targetLength, float duration)
{
	previousArmLength = m_SpringArm->TargetArmLength;
	targetArmlength = targetLength;
	if(previousArmLength == targetArmlength)
		return;;
	durationTransitionArmLength = duration;
	activeArmLengthTransition = true;
	timerArmlength = 0.0;
}

void ACar::OffsetWheelWithSuspension(int WheelIndex, bool isOnGround, float distance)
{
	const float WheelOffset = 15.f;
	float maxSuspensionLength = -0.5f * SuspensionLength;
	float Target = ((isOnGround  ? -distance : maxSuspensionLength) + WheelOffset + WheelRadii[WheelIndex]);

	FVector RelativeLocation = m_VehicleMesh->GetBoneLocationByName(WheelBoneNames[WheelIndex], EBoneSpaces::ComponentSpace);
	float DeltaTime = 0.016667f;
	float value = UKismetMathLibrary::FloatSpringInterp(RelativeLocation.Z, Target, WheelSpringStates[WheelIndex],
	                                                    WheelSpringStiffness, WheelSpringDamping, DeltaTime, 1.f);

	FVector WheelInitialLocation = WheelInitialLocations[WheelIndex];
	WheelInitialLocation.Z = value;
	m_VehicleMesh->SetBoneLocationByName(WheelBoneNames[WheelIndex], WheelInitialLocation, EBoneSpaces::ComponentSpace);
}

void ACar::UpdateBrushes()
{
	for (auto& brush : PaintBrushes)
	{
		ProjectBrushOnGround(brush);
		brush->MoveBrush();
	}
}


// Called every frame
void ACar::Tick(float DeltaTime)
{
	
	// if (!bIsSimulated)
	// 	return;
	//
	// TurnVelocityWithRotation(DeltaTime);
	// ComputeSuspension();
	// MoveCamera();
	
	
	// for (size_t i = 0; i < PaintBrushes.Num(); i++)
	// {
	// 	ProjectBrushOnGround(PaintBrushes[i], Wheels[i], WheelSpringStates[i]);
	//
	// 	PaintBrushes[i]->MoveBrush();
	// }
	
	// TiltCar();

	Super::Tick(DeltaTime);

	TransitionCamera(DeltaTime);
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

