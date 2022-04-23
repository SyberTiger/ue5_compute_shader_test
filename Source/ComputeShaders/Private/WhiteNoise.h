#pragma once

#include "CoreMinimal.h"

#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

struct  FWhiteNoiseCSParameters
{
	UTextureRenderTarget2D* RenderTarget;


	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FWhiteNoiseCSParameters() { }
	FWhiteNoiseCSParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
public:
	uint32 TimeStamp;
};

class WhiteNoise
{

public:
	WhiteNoise();



	void BeginRendering();
	void EndRendering();
	void UpdateParameters(FWhiteNoiseCSParameters& DrawParameters);

private:
	void Execute_RenderThread(FRDGBuilder& builder, const FSceneTextures& SceneTextures);



	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	FWhiteNoiseCSParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;



	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;

};