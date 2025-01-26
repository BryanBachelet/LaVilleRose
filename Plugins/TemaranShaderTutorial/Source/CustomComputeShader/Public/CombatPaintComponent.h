// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatPaintingManager.h"
#include "Components/ActorComponent.h"
#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"
#include "CombatPaintComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CUSTOMCOMPUTESHADER_API UCombatPaintComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatPaintComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual  void BeginDestroy() override;



	
public:

	UFUNCTION(BlueprintCallable)
	void InitializePaintingComponent(UCombatPaintingManager* combatPaintingManager) ;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	void PushPaintingShoot(bool isRed, TArray<FVector> verticesArray,FVector originPos,FRotator orientation);


	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsRendering  =false;
	FVector worldScale;
	FVector worldOffset;
private:
	UCombatPaintingManager* m_combatPaintingManager;
	
};
