// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPaintComponent.h"

#include "CombatPaintingManager.h"
#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"


// Sets default values for this component's properties
UCombatPaintComponent::UCombatPaintComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCombatPaintComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UCombatPaintComponent::BeginDestroy()
{
	if(FComputeShaderManager::Get())
	{
		//FComputeShaderManager::Get()->EndRendering();
		IsRendering =false;
		UE_LOG(LogTemp,Warning,TEXT(" End rendering Shape push "));

	}
	
	Super::BeginDestroy();
}


void UCombatPaintComponent::InitializePaintingComponent(UCombatPaintingManager* combatPaintingManager )
{
	
	m_combatPaintingManager  = combatPaintingManager;
	m_combatPaintingManager->InitializeCombatPaintingComponent(this);
}

// Called every frame
void UCombatPaintComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UCombatPaintComponent::PushPaintingShoot(bool isRed, TArray<FVector> verticesArray,FVector originPos,FRotator orientation)
{
	
	if(!FComputeShaderManager::Get() ||!IsRendering) return;

	// Set new vertex uv array
	
	
	FCombatPaintData combatData =  FCombatPaintData();
	int count =  verticesArray.Num();
	int offset = FComputeShaderManager::Get()->verticesCount;
	combatData.verticesCount = count;
	combatData.offset = offset;

	if(count+offset > MAX_VERTICES)
	{
		UE_LOG(LogTemp,Error,TEXT("Too much paint vertices"));
		return;
	}
	
	
	for (int i = 0 ; i <  verticesArray.Num() && i < MAX_VERTICES;i++)
	{
		FVector verticeOriented = orientation.RotateVector( verticesArray[i]);
		FVector newValue =  verticeOriented + originPos  + worldOffset;


		newValue.X = newValue.X / worldScale.X;
		newValue.Y = newValue.Y / worldScale.Y;
		newValue = (newValue + 0.5);	
			
		FVector data ;
		data = newValue;
		FComputeShaderManager::Get()->verticesArray[i + offset] = data;
		
	}

	combatData.IsRed = isRed;
	FComputeShaderManager::Get()->verticesCount += count;
	
	FComputeShaderManager::Get()->isFill = true;
	FComputeShaderManager::Get()->PushPaintShape(combatData);
	UE_LOG(LogTemp,Warning,TEXT(" Paint Shape brush push %s"),*GetOwner()->GetName());
	UE_LOG(LogTemp,Warning,TEXT(" Paint Shape brush push %i"),(int)(FComputeShaderManager::Get()->isFill));

	
}

