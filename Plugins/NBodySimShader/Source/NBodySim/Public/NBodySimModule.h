

#pragma once

#include "CoreMinimal.h"
#include "NBodySimCS.h"
#include "NBodySimTypesDefinitions.h"
#include "RenderGraphResources.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

// This struct contains all the data we need to pass from the game thread to compute on GPU.
struct FNBodySimParameters
{
public:

	uint32 shaderdebug;
	
	uint32 NumLinks;
	TArray<int> LinkOffsets;  // Holds the offset for each body
	TArray<int> LinkCounts;   // Holds the count of links for each body
	TArray<int> LinkIndices;  // Flat array containing all links
	TArray<float> LinkStrengths;  // Holds the strength of each link
	TArray<float> LinkBiases;     // Holds the bias of each link
	TArray<int> Linkinout;  

	


	TArray<FBodyData> Bodies;
	
	uint32 NumBodies;
	float GravityConstant;
	float CameraAspectRatio;
	float ViewportWidth;
	float DeltaTime;
	float alpha;
	float alphaS;
	
	
	FNBodySimParameters():
	NumLinks(0),
	NumBodies(0),
	GravityConstant(0),
	CameraAspectRatio(0),
	ViewportWidth(0),
	DeltaTime(0),
	alpha(1)
	{
	}
};

/*
 * Since we already have a module interface due to us being in a plugin, it's pretty handy to just use it
 * to interact with the renderer. It gives us the added advantage of being able to decouple any render
 * hooks from game modules as well, allowing for safer and easier cleanup.
 */
class NBODYSIM_API FNBodySimModule : public IModuleInterface
{
public:
	static inline FNBodySimModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FNBodySimModule>("NBodySim");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("NBodySim");
	}

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	// Call this when you want to hook onto the renderer and start drawing. The shader will be executed once per frame.
	void BeginRendering();

	// When you are done, call this to stop drawing.
	void EndRendering();
	void InitWithParameters(FNBodySimParameters& SimParameters);

	// Call this whenever you have new parameters to share. You could set this up to update different sets of properties at
	// different intervals to save on locking and GPU transfer time.
	void UpdateDeltaTime(float DeltaTime,float alpha=1);

	TArray<FVector3f> GetComputedPositions() { return OutputPositions; }
	TArray<float> GetComputedAlphas() { return Alpha; }


private:
	void PostResolveSceneColor_RenderThread(FRDGBuilder& Builder, const FSceneTextures& SceneTexture);


	void ComputeSimulation_RenderThread(FNBodySimParameters& SimParameters);
	
	
	FDelegateHandle OnPostResolvedSceneColorHandle;
	FCriticalSection RenderEveryFrameLock;

	FNBodySimParameters CachedNBodySimParameters;
	volatile bool bCachedParametersValid = false;

	FNBodySimCSBuffers CSBuffers;
	
	TArray<FVector3f> OutputPositions;
	TArray<float> Alpha;

};
