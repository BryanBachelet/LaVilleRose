// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ImageWriteBlueprintLibrary.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Engine/VolumeTexture.h"
#include "GameFramework/Actor.h"
#include "ResourceVolumeTex.generated.h"

UCLASS(Blueprintable)	
class EUROPERPG_API UResourceVolumeTex : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UResourceVolumeTex();

	UInputComponent* InputComponent = nullptr;
	UFUNCTION(BlueprintCallable)
	void LoadTexture() ;
	UFUNCTION(BlueprintCallable)
	void SaveTexture(UTextureRenderTargetVolume * texture);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:	
};
