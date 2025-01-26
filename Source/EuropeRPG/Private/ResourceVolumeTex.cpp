// Fill out your copyright notice in the Description page of Project Settings.

#include "ResourceVolumeTex.h"

#include "ImageWriteQueue.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Kismet/KismetRenderingLibrary.h"


// Sets default values
UResourceVolumeTex::UResourceVolumeTex()
{
	InputComponent = nullptr;
}

// Called when the game starts or when spawned
void UResourceVolumeTex::BeginPlay()
{
	Super::BeginPlay();

}

void UResourceVolumeTex::LoadTexture()
{
	UE_LOG(LogTemp,Log,TEXT("Call Load function"))
	//LoadObject<UVolumeTexture>(texture,"Tst","Test",LOAD);
}

void UResourceVolumeTex::SaveTexture(UTextureRenderTargetVolume * texture)
{
	UE_LOG(LogTemp,Log,TEXT("Call Save function"))


	
}


