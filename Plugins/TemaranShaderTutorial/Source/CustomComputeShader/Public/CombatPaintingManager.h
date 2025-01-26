// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"

#include "Components/ActorComponent.h"
#include "CombatPaintingManager.generated.h"

class UCombatPaintComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CUSTOMCOMPUTESHADER_API
UCombatPaintingManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatPaintingManager();

	virtual void BeginDestroy() override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	AActor* actorWorldScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = Painting)
	UTextureRenderTarget2D* renderTexture;

	UFUNCTION(BlueprintCallable)
		void InitializeCombatPaint();

	void InitializeCombatPaintingComponent(UCombatPaintComponent* CombatPaintComponent);


	UFUNCTION(BlueprintCallable)
	void ResetCombatManager();
	bool hasShoot;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool isInit;
	PaintShotData PaintShotData;
};
