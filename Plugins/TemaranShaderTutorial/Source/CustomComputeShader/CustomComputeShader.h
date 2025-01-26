// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"



class CUSTOMCOMPUTESHADER_API FCustomComputeShaderModule : public IModuleInterface
{
public:
	static inline FCustomComputeShaderModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FCustomComputeShaderModule>("CustomComputeShader");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("CustomComputeShader");
	}
};
