// Copyright Epic Games, Inc. All Rights Reserved.

#include "PostProcessingMaterial.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/RenderCore/Public/ShaderCore.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "FPostProcessingMaterialModule"

void FPostProcessingMaterialModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	const FString ProjectShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*ProjectShaderDirectory))
		IFileManager::Get().MakeDirectory(*ProjectShaderDirectory);
	AddShaderSourceDirectoryMapping(TEXT("/Project"), ProjectShaderDirectory);


	const FString PluginsShaderDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("PostProcessingMaterial/Shaders"));
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*PluginsShaderDirectory))
		IFileManager::Get().MakeDirectory(*PluginsShaderDirectory);
	AddShaderSourceDirectoryMapping(TEXT("/PostMaterial"), PluginsShaderDirectory);
}

void FPostProcessingMaterialModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPostProcessingMaterialModule, PostProcessingMaterial)
