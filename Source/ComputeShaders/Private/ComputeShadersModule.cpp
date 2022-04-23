#include "ComputeShadersModule.h"

#include "Modules/ModuleManager.h"

#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "GlobalShader.h"

IMPLEMENT_MODULE(FComputeShadersModule, ComputeShaders);

void FComputeShadersModule::StartupModule()
{
	const FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("ComputeShaders"));
	AddShaderSourceDirectoryMapping(FString("/CustomShaders"), ShaderDirectory);
}

void FComputeShadersModule::ShutdownModule()
{
	// Put your module termination code here
}