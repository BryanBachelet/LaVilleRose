#include "ComputeShaderDeclaration.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include <Engine/TextureRenderTargetVolume.h>
#include <unordered_map>
#include <iostream>
#include "Modules/ModuleManager.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 8


/// <summary>
/// Internal class thet holds the parameters and connects the HLSL Shader to the engine
/// </summary>
class FPainterCS : public FGlobalShader
{
public:
	//Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FPainterCS);
	//Tells the engine that this shader uses a structure for its parameters
	/// <summary>
	/// DECLARATION OF THE PARAMETER STRUCTURE
	/// The parameters must match the parameters in the HLSL code
	/// For each parameter, provide the C++ type, and the name (Same name used in HLSL code)
	/// </summary>
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_UAV(RWTexture3D<float4>, VolumeTexture)
	SHADER_PARAMETER_UAV (RWStructuredBuffer<FPainterCSParams>,BrushBuffer)
	SHADER_PARAMETER(int,computeMax)
	SHADER_PARAMETER(int, cubeSizeThread)
	SHADER_PARAMETER(float, offsetUV)
	SHADER_PARAMETER(FVector, Dimensions)
	SHADER_PARAMETER(float, WorldTime)
	END_SHADER_PARAMETER_STRUCT()

	SHADER_USE_PARAMETER_STRUCT(FPainterCS, FGlobalShader)
	;

public:
	//Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	//Modifies the compilations environment of the shader
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for NUM_THREADS_PER_GROUP_DIMENSION
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
		OutEnvironment.SetDefine(TEXT("MAX_BRUSH"), MAX_BRUSH);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//                        ShaderType              ShaderPath             Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FPainterCS, "/CustomShaders/PainterCS.usf", "MainComputeShader", SF_Compute);



// Global Shader for combat painting 
class FCombatPainter : public FGlobalShader
{
public:
	
	//Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FCombatPainter);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, renderTarget)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<FCombatPaintData>,paintShapeData)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<float3>,verticesData)
	SHADER_PARAMETER(FVector, Dimensions)	
	SHADER_PARAMETER(int , brushCount)
	SHADER_PARAMETER(int , isFirstTime)
	
	END_SHADER_PARAMETER_STRUCT()

	SHADER_USE_PARAMETER_STRUCT(FCombatPainter, FGlobalShader)
	;


	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
		OutEnvironment.SetDefine(TEXT("MAX_VERTICES"),MAX_VERTICES);
	}
	
	
};

// This will tell the engine to create the shader and where the shader entry point is.
//                        ShaderType              ShaderPath             Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FCombatPainter, "/CustomShaders/CombatPainterCS.usf", "MainComputeShader", SF_Compute); 

//Static members
FComputeShaderManager* FComputeShaderManager::instance = nullptr;

FBrushData::FBrushData()
{
}

void FComputeShaderManager::InitializeCombatPainter(UTextureRenderTarget2D* pTexture, FVector scale)
{
	EPixelFormat PixelFormat = PF_B8G8R8A8;

	renderTarget2D = pTexture;
	renderTarget2D->bCanCreateUAV = true;

	renderTarget2D->InitCustomFormat(1024,1024,PixelFormat,true);

	FlushRenderingCommands();
 
	if(!renderTarget2D ||  !renderTarget2D->Resource|| !renderTarget2D->Resource->TextureRHI)
	{
		return;
	}

	RenderTargetUAV =  RHICreateUnorderedAccessView(renderTarget2D->Resource->TextureRHI);

	// Adding brush data buffer to compute shader
	int32 InputSize = sizeof(FCombatPaintData);
	
	combatPainArray.SetNumUninitialized(MAX_COMBAT_BRUSH);

	for(int i = 0 ; i < MAX_COMBAT_BRUSH ; i++)
	{
		combatPainArray[i].verticesCount = 0;
	}
	
	
	FRHIResourceCreateInfo info = FRHIResourceCreateInfo(TEXT("CombatPaintData"));
	info.ResourceArray = &combatPainArray;

	combatPaintRHI = RHICreateStructuredBuffer(InputSize,MAX_COMBAT_BRUSH * InputSize,BUF_UnorderedAccess | BUF_ShaderResource | BUF_SourceCopy, info);
	PUBufferTargetCombat = RHICreateUnorderedAccessView(combatPaintRHI,false,false);

	
	// Adding brush data buffer to compute shader
	InputSize = sizeof(FVector);
	
	FRHIResourceCreateInfo info2 = FRHIResourceCreateInfo(TEXT("CombatPaintData"));
	info2.ResourceArray = &verticesArray;

	verticesArray.SetNumUninitialized(MAX_VERTICES);
	
	verticesPaintRHI = RHICreateStructuredBuffer(InputSize,MAX_VERTICES * InputSize,BUF_UnorderedAccess | BUF_ShaderResource | BUF_SourceCopy, info2);
	PUBufferTargetVertices = RHICreateUnorderedAccessView(verticesPaintRHI,false,false);
	
	isInitialized =	true;
	bCachedParamsAreValid = true;

	isCurrentlyExploPainter = false;

	combatPainArray.SetNumUninitialized(MAX_COMBAT_BRUSH);

	for(int i = 0 ; i < MAX_COMBAT_BRUSH ; i++)
	{
		combatPainArray[i].verticesCount = 0;
	}
	
	verticesArray.SetNumUninitialized(MAX_VERTICES);
	isCombatPaitingOn =true;
	isFirstTimeCombat = true;
	if(!isExploPaintingOn) InitRendering();
}

void FComputeShaderManager::InitializeExploPaint(UTextureRenderTargetVolume* pTexture, FVector scale,
                                                 float brushWorldSize)
{
	isCurrentlyExploPainter = true;
	isExploPaintingOn = true;
	EPixelFormat PixelFormat = PF_A16B16G16R16;

	VolumeRenderTarget = pTexture;
	VolumeRenderTarget->bCanCreateUAV = true;

	VolumeRenderTarget->Init(1024, 1024, 16, PixelFormat);

	pixelSizeAroundBrush = (brushWorldSize * 1024)/(scale.X * 100);

	computeGroupSize = pixelSizeAroundBrush/NUM_THREADS_PER_GROUP_DIMENSION;
	
	UE_LOG(LogTemp,Error,TEXT("Temp: Test function existing"))
	
	FlushRenderingCommands();

	if (!VolumeRenderTarget || !VolumeRenderTarget->Resource ||
		!VolumeRenderTarget->Resource->TextureRHI )
	{
		UE_LOG(LogTemp,Error,TEXT("Volume not init"))
		// Return if anything was not initialized.
		return;
	}
	
	VolumeRenderTargetUAV = RHICreateUnorderedAccessView(VolumeRenderTarget->Resource->TextureRHI);

	// Adding brush data buffer to compute shader
	int32 InputSize = sizeof(FBrushData);
	
	mappedBrush.SetNumUninitialized(8);
	
	UE_LOG(LogTemp,Warning,TEXT("Max brush use %i"),mappedBrush.Num());
	for(int i  = 0 ; i<mappedBrush.Num();i++)
	{
		mappedBrush[i].isActive = false;
	}

	UE_LOG(LogTemp,Warning,TEXT("Max brush use %i"),mappedBrush.Num());
	FRHIResourceCreateInfo info = FRHIResourceCreateInfo(TEXT("SoftBodyMainBuf"));
	info.ResourceArray = &mappedBrush;

	brushDataRHI = RHICreateStructuredBuffer(InputSize,MAX_BRUSH * InputSize,BUF_UnorderedAccess | BUF_ShaderResource | BUF_SourceCopy, info);
	PUBufferTarget = RHICreateUnorderedAccessView(brushDataRHI,false,false);
	
	isInitialized = true;
	bCachedParamsAreValid = true;
	
	mappedBrush.SetNumUninitialized(8);
	
	for(int i  = 0 ; i<mappedBrush.Num();i++)
	{
		mappedBrush[i].isActive = false;
	}
}



//Begin the execution of the compute shader each frame
void FComputeShaderManager::BeginBrushRendering(USceneComponent* paintBrushRef,int& indexBrush)
{
	cachedParams.BrushToParams.Add(paintBrushRef, new FPainterCSParams());
	indexBrush = -1;
	for (int i = 0 ; i < mappedBrush.Num() ; i++)
	{
		if(!mappedBrush[i].isActive)
		{
			indexBrush = i;
			break;
		}
	}
	if(indexBrush == -1)
	{
		UE_LOG(LogTemp,Error,TEXT("Max brush use %i"),mappedBrush.Num());
		UE_LOG(LogTemp,Error,TEXT("Is Init %i"),(int)isInitialized);
		return;
	}

	cachedParams.BrushIndex.Add(paintBrushRef,indexBrush);
	
	mappedBrush[indexBrush].isActive = true;

	InitRendering();
}

//Stop the compute shader execution
void FComputeShaderManager::EndBrushRendering(USceneComponent* paintBrushRef)
{
	cachedParams.BrushToParams.Remove(paintBrushRef);
	int index = -1;
	if(cachedParams.BrushIndex.Contains(paintBrushRef))
		index = cachedParams.BrushIndex[paintBrushRef];
	
	if(index != -1 ) mappedBrush[index].isActive =false;
	
	cachedParams.BrushIndex.Remove(paintBrushRef);

	EndRendering();
	
}

void FComputeShaderManager::InitRendering()
{
		
	//If the handle is already initalized and valid, no need to do anything
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	//Get the Renderer Module and add our entry to the callbacks so it can be executed each frame after the scene rendering is done
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FComputeShaderManager::Execute_RenderThread);
	}
}

void FComputeShaderManager::EndRendering()
{
	//If the handle is not valid then there's no cleanup to do
	if (!OnPostResolvedSceneColorHandle.IsValid() || cachedParams.BrushToParams.Num() != 0)
	{
		
		return;
	}


	isExploPaintingOn = false;
	//Get the Renderer Module and remove our entry from the ResolvedSceneColorCallbacks
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}
	UE_LOG(LogTemp,Warning,TEXT(" End Render"));

	OnPostResolvedSceneColorHandle.Reset();
}

void FComputeShaderManager::EndCombatRendering()
{
	isCurrentlyExploPainter =true;
	isCombatPaitingOn =false;
}

/// <summary>
///	Allow to register new paint shape to paint
/// </summary>
void FComputeShaderManager::PushPaintShape(FCombatPaintData data)
{

	UE_LOG(LogTemp,Warning,TEXT(" Paint Shape push"));

	if(BrushCombatCount >=  MAX_COMBAT_BRUSH)
	{
		UE_LOG(LogTemp,Error,TEXT(" Max combat paint already sended"));
		return;
	}
	
	combatPainArray[BrushCombatCount] = data;
	BrushCombatCount++;
	
}

/// <summary>
/// Creates an instance of the shader type parameters structure and fills it using the cached shader manager parameter structure
/// Gets a reference to the shader type from the global shaders map
/// Dispatches the shader using the parameter structure instance
/// </summary>
void FComputeShaderManager::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	//If there's no cached parameters to use, skip
	//If no Render Target is supplied in the cachedParams, skip
	if (!(bCachedParamsAreValid))
	{
		return;
	}

	//Render Thread Assertion
	check(IsInRenderingThread());
	
	//Unbind the previously bound render targets
	//UnbindRenderTargets(RHICmdList);

	if(isCurrentlyExploPainter) UpdateExploShader(RHICmdList,SceneContext);
	else UpdateCombatShader(RHICmdList,SceneContext);

}

void FComputeShaderManager::UpdateExploShader(FRHICommandListImmediate& RHICmdList, FSceneRenderTargets& SceneContext)
{
	//Fill the shader parameters structure with tha cached data supplied by the client
	FPainterCS::FParameters PassParameters;
	PassParameters.VolumeTexture = VolumeRenderTargetUAV;
	PassParameters.BrushBuffer = PUBufferTarget;
	PassParameters.computeMax = pixelSizeAroundBrush * pixelSizeAroundBrush;
	PassParameters.cubeSizeThread = computeGroupSize;
	PassParameters.WorldTime = WorldTime;
	PassParameters.offsetUV = 1.0 /VolumeRenderTarget->SizeX;
	PassParameters.Dimensions = FVector(VolumeRenderTarget->SizeX, VolumeRenderTarget->SizeY, VolumeRenderTarget->SizeZ);

	//Get a reference to our shader type from global shader map
	TShaderMapRef<FPainterCS> painterCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));


	//Dispatch the compute shader
	FComputeShaderUtils::Dispatch(RHICmdList, painterCS, PassParameters,
		FIntVector(
			computeGroupSize,
			computeGroupSize,
			1));
		
	//Copy shader's output to the render target provided by the client
	RHICmdList.Transition(FRHITransitionInfo(VolumeRenderTargetUAV, ERHIAccess::UAVCompute, ERHIAccess::UAVGraphics));
	

	void* StructuredBufferData = RHILockStructuredBuffer(brushDataRHI, 0, mappedBrush.Num() * sizeof(FBrushData), RLM_WriteOnly);
	FMemory::Memcpy(StructuredBufferData, mappedBrush.GetData(), mappedBrush.Num() * sizeof(FBrushData));
	RHIUnlockStructuredBuffer(brushDataRHI);
}

void FComputeShaderManager::UpdateCombatShader(FRHICommandListImmediate& RHICmdList, FSceneRenderTargets& SceneContext)
{

	
	if(!isFill && !isFirstTimeCombat) return;
	FCombatPainter::FParameters PassParameters;

	PassParameters.renderTarget = RenderTargetUAV;
	PassParameters.Dimensions = FVector(renderTarget2D->SizeX,renderTarget2D->SizeY,1);
	PassParameters.brushCount = BrushCombatCount;
	PassParameters.paintShapeData = PUBufferTargetCombat;
	PassParameters.verticesData = PUBufferTargetVertices;
	PassParameters.isFirstTime = (int)isFirstTimeCombat;
	TShaderMapRef<FCombatPainter> combatPainter(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	
	void* StructuredBufferData = RHILockStructuredBuffer(combatPaintRHI, 0, combatPainArray.Num() * sizeof(FCombatPaintData), RLM_WriteOnly);
	FMemory::Memcpy(StructuredBufferData, combatPainArray.GetData(), combatPainArray.Num() * sizeof(FCombatPaintData));
	RHIUnlockStructuredBuffer(combatPaintRHI);

	void* StructuredBufferData2 = RHILockStructuredBuffer(verticesPaintRHI, 0, verticesArray.Num() * sizeof(FVector), RLM_WriteOnly);
	FMemory::Memcpy(StructuredBufferData2, verticesArray.GetData(), verticesArray.Num() * sizeof(FVector));
	RHIUnlockStructuredBuffer(verticesPaintRHI);

	FComputeShaderUtils::Dispatch(RHICmdList, combatPainter, PassParameters,
		FIntVector(
			FMath::DivideAndRoundUp(renderTarget2D->SizeX,NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(renderTarget2D->SizeX,NUM_THREADS_PER_GROUP_DIMENSION),
			1));
		
	//Copy shader's output to the render target provided by the client
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetUAV, ERHIAccess::UAVCompute, ERHIAccess::UAVGraphics));

	
	
	isFill = false;
	for(int i = 0 ; i < BrushCombatCount; i++)
	{
		combatPainArray[i].verticesCount = 0 ;
		combatPainArray[i].offset = 0 ;
	}
	//verticesArray.Reset();
	verticesCount = 0;
	BrushCombatCount = 0;
	isFirstTimeCombat = false;
	UE_LOG(LogTemp,Warning,TEXT(" Paint Shape brush count is reset %i"),BrushCombatCount);
}
