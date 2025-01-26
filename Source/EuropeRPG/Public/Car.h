// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Car.generated.h"

class UPoseableMeshComponent;
class USpringArmComponent;
class UPaintBrushComponent;

UCLASS()
class EUROPERPG_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void InitializeWheelLocations();

	UFUNCTION(BlueprintCallable)
	void InitializePainting();

	// Polish
	UFUNCTION(BlueprintCallable)
	void TurnVelocityWithRotation(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ComputeSuspension();

	UFUNCTION(BlueprintCallable)
	void MoveCamera();

	UFUNCTION(BlueprintCallable)
	void TiltCar();

	UFUNCTION(BlueprintCallable)
	void TurnWheelWhenSteering();

	UFUNCTION(BlueprintCallable)
	void ProjectWheelsOnGround();

	UFUNCTION(BlueprintCallable)
	void OffsetWheelWithSuspension(int WheelIndex, bool isOnGround, float distance);

	UFUNCTION(BlueprintCallable)
	void UpdateBrushes();

	UFUNCTION(BlueprintCallable)
	void ProjectBrushOnGround(UPaintBrushComponent* Brush);

	void TransitionCamera(float deltaTime);
	
	UFUNCTION(BlueprintCallable)
	void ActiveCameraTransitonArmLength(float targetLength, float duration);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	FVector WorldScale = FVector(20000.f, 20000.f, 3000.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
	FVector WorldOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Suspension")
	float SuspensionLength = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Suspension")
	float SuspensionForce = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheel")
	float WheelSpringStiffness = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheel")
	float WheelSpringDamping = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheel")
	float WheelSteeringAngleMultiplier = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheel")
	TArray<FVector> WheelInitialLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car|Wheel")
	TArray<FName> WheelBoneNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheel")
	TArray<float> WheelRadii ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Wheel")
	TArray<FFloatSpringState> WheelSpringStates;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Tilt")
	float TiltSpringStiffness = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Tilt")
	float TiltSpringDamping = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car|Tilt")
	float DesiredTiltMultiplier = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Painting")
	AActor* PaintScaleActor = nullptr;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TransitionCamara")
	float durationTransitionArmLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransitionCamera")
	float targetArmlength;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category  = "TransitionCamera")
	float previousArmLength;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= "TransitionCamera")
	bool activeArmLengthTransition = false;
 	
	
protected:
	
	float timerArmlength = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Car")
	bool bIsGrounded = false;

	UPROPERTY(BlueprintReadWrite, Category = "Car")
	float GroundFriction = 0.1f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Car")
	float ThrustDirection = 0.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Car|Tilt")
	float CurrentTilt = 0.f;

	FFloatSpringState TiltSpring;
	
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	bool bCameraFollowsPlayer = true;
	
	UPROPERTY(BlueprintReadOnly, Category = "Paint")
	TArray<class UPaintBrushComponent*> PaintBrushes;

private:
	UStaticMeshComponent* m_CollisionMesh;

	USpringArmComponent* m_SpringArm;

	UPoseableMeshComponent* m_VehicleMesh;
};
