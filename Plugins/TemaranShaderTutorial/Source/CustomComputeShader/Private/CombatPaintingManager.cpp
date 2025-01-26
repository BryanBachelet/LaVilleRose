// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPaintingManager.h"
#include "CombatPaintComponent.h"
#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UCombatPaintingManager::UCombatPaintingManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCombatPaintingManager::BeginDestroy()
{
	if(isInit)
	{
		UE_LOG(LogTemp,Warning,TEXT(" End rendering Shape manager "));
		FComputeShaderManager::Get()->EndCombatRendering();
		if(	!FComputeShaderManager::Get()->isExploPaintingOn)
		{
			FComputeShaderManager::Get()->EndRendering();
		}
	}
	Super::BeginDestroy();
	
}


void UCombatPaintingManager::InitializeCombatPaint()
{
	
	if(actorWorldScale == nullptr)
	{
		UE_LOG(LogTemp, Error , TEXT("No Paint Volume found"))
		return;
	}

	UE_LOG(LogTemp, Log , TEXT(" Paint Volume found"))
	FVector scale =  actorWorldScale->GetActorScale3D() * 100;

	FComputeShaderManager::Get()->InitializeCombatPainter(renderTexture,scale);
	isInit = true;
}

void UCombatPaintingManager::InitializeCombatPaintingComponent(UCombatPaintComponent* CombatPaintComponent)
{
	
	if(actorWorldScale == nullptr)
	{
		UE_LOG(LogTemp, Error , TEXT("No Paint Volume found. Can't init paint volume"))
		return;
	}
	
	CombatPaintComponent->worldScale = actorWorldScale->GetActorScale() * 100;
	CombatPaintComponent->worldOffset = actorWorldScale->GetActorLocation();

	CombatPaintComponent->IsRendering = true;
}

void UCombatPaintingManager::ResetCombatManager()
{
	UE_LOG(LogTemp,Warning,TEXT(" End rendering Shape manager "));
	FComputeShaderManager::Get()->EndCombatRendering();
}
