// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBrushComponent.h"


#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"


#include "DrawDebugHelpers.h"
#include "RHIResources.h"

class UTextureRenderTargetVolume;

//FComputeShaderManager* UPaintBrushComponent::GetComputeShaderManager() {
//	return FComputeShaderManager::Get();
//}

// Sets default values for this component's properties
UPaintBrushComponent::UPaintBrushComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPaintBrushComponent::BeginPlay()
{
	Super::BeginPlay();

	//get texture reference
	//FWhiteNoiseCSManager::Get()->InitializeTexture();
} 
void UPaintBrushComponent::BeginDestroy() {
	EndRendering();
	Super::BeginDestroy();
}

void UPaintBrushComponent::BeginRendering() 
{
	if (FComputeShaderManager::Get()) {
		FComputeShaderManager::Get()->BeginBrushRendering(this,indexBrush);
		isRendering = true;
	}
}
void UPaintBrushComponent::EndRendering()
{
	if (FComputeShaderManager::Get()) 
	{
		FComputeShaderManager::Get()->EndBrushRendering(this);
		isRendering = false;
	}
}

void UPaintBrushComponent::InitializeValues(UMaterialInstanceDynamic* pMaterialInstance, FVector pWorldScale, FVector pWorldOffset)
{
	MaterialInstance = pMaterialInstance;
	WorldScale = pWorldScale * 100;
	WorldOffset = pWorldOffset;
}



void UPaintBrushComponent::MoveBrush()
{
	FVector brushLocation = GetComponentTransform().GetLocation() - WorldOffset;
	BrushUV = CalculationUV(brushLocation);
	FVector locationOrigin =  FVector::ZeroVector;
	FRotator originRotation =   FRotator(0,GetComponentRotation().Yaw,0);

	locationOrigin = brushLocation +  (-FVector::XAxisVector + -FVector::YAxisVector) * brushImpactDistance/2.0f;

	originBrushUV = CalculationUV(locationOrigin);

}

FVector UPaintBrushComponent::CalculationUV(FVector location)
{
	
	FVector newLocation =  (location) ;
	newLocation =  newLocation/WorldScale;
	newLocation =  (newLocation + 0.5);


	return 	newLocation;
}

// Called every frame
void UPaintBrushComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (isRendering) 
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


		if(indexBrush == -1) return;

		//Update parameters
		FComputeShaderManager::Get()->mappedBrush[indexBrush].BrushUV = BrushUV;
		FComputeShaderManager::Get()->mappedBrush[indexBrush].originUV = originBrushUV;
		FComputeShaderManager::Get()->mappedBrush[indexBrush].BrushSize = BrushSize;
		FComputeShaderManager::Get()->mappedBrush[indexBrush].IsBlue = IsBlue ? 1:0; 
		FComputeShaderManager::Get()->mappedBrush[indexBrush].isActive = true ; 
	}
}

