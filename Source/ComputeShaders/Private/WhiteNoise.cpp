#include "WhiteNoise.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderTargetPool.h"
#include "RHI.h"



#include "Modules/ModuleManager.h"



#define NUM_THREADS_PER_GROUP_DIMENSION 8

class FWhiteNoiseCS : public FGlobalShader
{

    DECLARE_GLOBAL_SHADER(FWhiteNoiseCS)
    SHADER_USE_PARAMETER_STRUCT(FWhiteNoiseCS, FGlobalShader)



        BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
        //SHADER_PARAMETER(FVector2D, Dimensions)
        SHADER_PARAMETER(UINT, TimeStamp)
        END_SHADER_PARAMETER_STRUCT()



    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }



    static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);



        //We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for 
		NUM_THREADS_PER_GROUP_DIMENSION;
            OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
        OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
        OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
    }

};

IMPLEMENT_GLOBAL_SHADER(FWhiteNoiseCS, "/CustomShaders/WhiteNoise.usf", "MainCS", SF_Compute);

WhiteNoise::WhiteNoise()
{
}


void WhiteNoise::BeginRendering()
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
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &WhiteNoise::Execute_RenderThread);
	}
}

//Stop the compute shader execution
void WhiteNoise::EndRendering()
{
	//If the handle is not valid then there's no cleanup to do
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	//Get the Renderer Module and remove our entry from the ResolvedSceneColorCallbacks
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();

}

void WhiteNoise::UpdateParameters(FWhiteNoiseCSParameters& DrawParameters)
{
	cachedParams = DrawParameters;
	bCachedParamsAreValid = true;
}


void WhiteNoise::Execute_RenderThread(FRDGBuilder& builder, const FSceneTextures& SceneTextures)
{

	FRHICommandListImmediate& RHICmdList = builder.RHICmdList;

	//If there's no cached parameters to use, skip
	//If no Render Target is supplied in the cachedParams, skip
	if (!(bCachedParamsAreValid && cachedParams.RenderTarget))
	{
		return;
	}

	//Render Thread Assertion
	check(IsInRenderingThread());

	//If the render target is not valid, get an element from the render target pool by supplying a Descriptor
	if (!ComputeShaderOutput.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Compute Shader Output Not Valid; re-generating"));
		FIntPoint Size = cachedParams.GetRenderTargetSize();
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(Size,cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		//FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(cachedParams.GetRenderTargetSize(), cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		ComputeShaderOutputDesc.DebugName = TEXT("WhiteNoiseCS_Output_RenderTarget1");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("WhiteNoiseCS_Output_RenderTarget2"));

	}

	//Specify the resource transition, we're executing this in post scene rendering so we set it to Graphics to Compute
	ERHIAccess transitionType = ERHIAccess::SRVMask;

	//RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, ComputeShaderOutput->GetRenderTargetItem().UAV);
	RHICmdList.TransitionResource(transitionType, cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI);

	FWhiteNoiseCS::FParameters PassParameters;
	PassParameters.OutputTexture = ComputeShaderOutput->GetRenderTargetItem().UAV;
	//PassParameters.Dimensions = FVector2D(cachedParams.GetRenderTargetSize().X, cachedParams.GetRenderTargetSize().Y);
	PassParameters.TimeStamp = cachedParams.TimeStamp;

	//Get a reference to our shader type from global shader map
	TShaderMapRef<FWhiteNoiseCS> whiteNoiseCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	//Dispatch the compute shader
	FComputeShaderUtils::Dispatch(RHICmdList, whiteNoiseCS, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(cachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(cachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
			1));

	//Copy shader's output to the render target provided by the client
	RHICmdList.CopyTexture(ComputeShaderOutput->GetRenderTargetItem().ShaderResourceTexture, cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());

	//Unbind the previously bound render targets

	GRenderTargetPool.FreeUnusedResource(ComputeShaderOutput);
}

