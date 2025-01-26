// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingManager.h"
#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"

// Sets default values for this component's properties
UPaintingManager::UPaintingManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UPaintingManager::InitializePaintTexture()
{
	if(actorWorldScale == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("No Paint volume found"))
		return;
	}
	
	FVector scale = actorWorldScale->GetActorScale3D();	
	
	FComputeShaderManager::Get()->InitializeExploPaint(RenderTarget,scale,brushSizeAround);
}

void UPaintingManager::LoadPaintTexture(UTextureRenderTargetVolume* pTexture) {

	if(actorWorldScale == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("No Paint volume found"))
		return;
	}
	FVector scale = actorWorldScale->GetActorScale3D();
	FComputeShaderManager::Get()->InitializeExploPaint(pTexture,scale,brushSizeAround);
}

void UPaintingManager::UpdateWorldTime(float pWorldTime)
{
	FComputeShaderManager::Get()->WorldTime = pWorldTime;
}

