// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "PaintBrushComponent.generated.h"

class UTextureRenderTargetVolume;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMCOMPUTESHADER_API UPaintBrushComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	//UFUNCTION(BlueprintPure, Category = "Paint")
	//	static class FComputeShaderManager* GetComputeShaderManager();
	// Sets default values for this component's properties
	UPaintBrushComponent();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		float BrushSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		FVector BrushUV;

	UPROPERTY(EditAnywhere,Category=ShaderDemo)
	FVector originBrushUV;
	UPROPERTY(EditAnywhere,Category=ShaderDemo)
	float brushImpactDistance = 1000;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		bool IsBlue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		FVector WorldScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		FVector WorldOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		UMaterialInstanceDynamic* MaterialInstance;

	bool isRendering;
	int indexBrush =-1;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	

	FVector CalculationUV(FVector location);

public:
	UFUNCTION(BlueprintCallable)
	void BeginRendering();
	UFUNCTION(BlueprintCallable)
	void EndRendering();

	UFUNCTION(BlueprintCallable)
	void InitializeValues(UMaterialInstanceDynamic* pMaterialInstance, FVector pWorldScale, FVector pWorldOffset);
	UFUNCTION(BlueprintCallable)
	void MoveBrush();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
