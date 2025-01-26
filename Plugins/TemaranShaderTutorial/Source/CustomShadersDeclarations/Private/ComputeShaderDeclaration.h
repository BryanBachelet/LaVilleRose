#pragma once

#include "CoreMinimal.h"
#include <unordered_map>
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "RHIResources.h"

class UTextureRenderTargetVolume;


#define MAX_BRUSH 8
#define MAX_VERTICES 5000
#define MAX_COMBAT_BRUSH 10

class FBrushData
{
public:
	FVector BrushUV;
	FVector originUV;
	float BrushSize;
	int IsBlue;
	int32 isActive;

	FBrushData();
	
};

	class FCombatPaintData
	{

		public:
		int offset;
		int verticesCount;
		int IsRed;
		
		
	};



struct PaintShotData
{
	TArray<FVector> verticesUV;
	bool isRed;
};

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
class FPainterCSParams {
public:
	int count;
};

struct  FPainterCSParameters
{
public:
	TMap<USceneComponent* , FPainterCSParams*> BrushToParams;
	TMap<USceneComponent* , int> BrushIndex;
};


/// <summary>
/// A singleton Shader Manager for our Shader Type
/// </summary>
class CUSTOMSHADERSDECLARATIONS_API FComputeShaderManager
{
public:
	//Get the instance
	static FComputeShaderManager* Get()
	{
		if (!instance) {
			instance = new FComputeShaderManager();
			UE_LOG(LogTemp,Warning,TEXT(" W : Singleton instantiate "))
		}

		return instance;
	};


	void InitializeCombatPainter(UTextureRenderTarget2D* pTexture, FVector scale);
	void InitializeExploPaint(UTextureRenderTargetVolume* pTexture,FVector scale, float brushWorldSize);

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginBrushRendering(USceneComponent* paintBrushRef,int& indexBrush);

	// Stops compute shader execution
	void EndBrushRendering(USceneComponent* paintBrushRef);


	void InitRendering();
	void EndRendering();

	void EndCombatRendering();

	bool isCurrentlyExploPainter ;
	bool isExploPaintingOn;
	bool isCombatPaitingOn;
	bool isFirstTimeCombat;

	class UTextureRenderTargetVolume* VolumeRenderTarget = nullptr;
	FUnorderedAccessViewRHIRef VolumeRenderTargetUAV;

	UTextureRenderTarget2D* renderTarget2D =nullptr;
	FUnorderedAccessViewRHIRef RenderTargetUAV;

	int pixelSizeAroundBrush;
	int computeGroupSize;
	int brushCount ;

	
	TResourceArray<FBrushData> mappedBrush ;
	FStructuredBufferRHIRef brushDataRHI ;
	FUnorderedAccessViewRHIRef PUBufferTarget;

	// Combat variables

	TResourceArray<FCombatPaintData> combatPainArray;
	FStructuredBufferRHIRef combatPaintRHI ;
	FUnorderedAccessViewRHIRef PUBufferTargetCombat;

	TResourceArray<FVector> verticesArray;
	FStructuredBufferRHIRef verticesPaintRHI ;
	FUnorderedAccessViewRHIRef PUBufferTargetVertices;

	bool isFill = true;
	int BrushCombatCount;
	int verticesCount;
	float WorldTime;
	
	void PushPaintShape(FCombatPaintData data);
	
	//Cached Shader Manager Parameters
	FPainterCSParameters cachedParams;
private:
	//Private constructor to prevent client from instanciating
	FComputeShaderManager() = default;

	//The singleton instance
	static FComputeShaderManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;


	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;
	volatile bool isInitialized;
public:
	void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);

	void UpdateExploShader(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
	void UpdateCombatShader(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
};
