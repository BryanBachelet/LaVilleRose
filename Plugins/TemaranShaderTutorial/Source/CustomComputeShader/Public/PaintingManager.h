// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PaintingManager.generated.h"

class UTextureRenderTargetVolume;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMCOMPUTESHADER_API UPaintingManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPaintingManager();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	AActor* actorWorldScale;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float brushSizeAround = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Painting)
		UTextureRenderTargetVolume* RenderTarget;

	UFUNCTION(BlueprintCallable)
		void InitializePaintTexture();
	UFUNCTION(BlueprintCallable)
		void LoadPaintTexture(UTextureRenderTargetVolume* pTexture);

	UFUNCTION(BlueprintCallable)
		void UpdateWorldTime(float pWorldTime);
};
