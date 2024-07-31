// 未经本人允许禁止私自传播


#include "PostStrokeAndFill.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AssetRegistry/AssetData.h"
#include "Materials/Material.h"

APostStrokeAndFill::APostStrokeAndFill()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	RootComponent = PostProcess;

	PostProcess->Settings.MotionBlurAmount = 0;
	PostProcess->Settings.MotionBlurMax = 0;

	// 资产注册模块
	if (const FAssetRegistryModule* AssetRegistryModule = FModuleManager::Get().GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
	{
		// 资产注册
		AssetRegistry = &AssetRegistryModule->Get();
	}
}

void APostStrokeAndFill::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void APostStrokeAndFill::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GetMaterials();
	AddPostMaterial();
}
#endif

void APostStrokeAndFill::AddPostMaterial()
{
	if (Materials.Num() == PostSettings.Num() && PostSettings.IsValidIndex(0))
	{
		PostProcessMaterial.Array.Reset();

		for (int i = 0; i <= PostSettings.Num() - 1; i++)
		{
			if (Materials.Num() <= PostSettings.Num())
			{
				FWeightedBlendable New = FWeightedBlendable(1.f, Cast<UObject>(SetPostMaterialProperties(Materials[i], PostSettings[i])));
				PostProcessMaterial.Array.Add(New);
			}
		}

		if (PostProcessMaterial.Array.IsValidIndex(0))
		{
			if (CustomPostMaterial.Array.IsValidIndex(0))
			{
				for (int i = 0; i <= CustomPostMaterial.Array.Num() - 1; i++)
				{
					PostProcessMaterial.Array.Add(CustomPostMaterial.Array[i]);
				}
			}

			PostProcess->Settings.WeightedBlendables = PostProcessMaterial;
		}
	}
	else
	{
		PostProcessMaterial.Array.Reset();
		if (CustomPostMaterial.Array.IsValidIndex(0))
		{
			for (int i = 0; i <= CustomPostMaterial.Array.Num() - 1; i++)
			{
				PostProcessMaterial.Array.Add(CustomPostMaterial.Array[i]);
			}
		}
		PostProcess->Settings.WeightedBlendables = PostProcessMaterial;
	}
}

UMaterialInterface* APostStrokeAndFill::SetPostMaterialProperties(UMaterialInterface* Material, const FPost_Struct& Post_Struct)
{
	if (UMaterialInstanceDynamic* LastMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Material))
	{
		// 开启材质
		LastMaterial->SetScalarParameterValue("EnableMaterial", Post_Struct.bEnableMaterial);
		// 自定义摸具值
		LastMaterial->SetScalarParameterValue("CustomDepthStencilValue", Post_Struct.CustomDepthStencilValue);

		// 另一个后期材质
		if (Post_Struct.bAnother == true)
		{
			// 外描边
			LastMaterial->SetScalarParameterValue("OuterStrokes", Post_Struct.bOuterStrokes);
			// 分辨率
			LastMaterial->SetVectorParameterValue("Resolution", FLinearColor(Post_Struct.Resolution.X, Post_Struct.Resolution.Y, 0.f, 0.f));
		}

		// 开启描边
		LastMaterial->SetScalarParameterValue("OpenStroke", Post_Struct.bOutline);
		// 添加填充
		LastMaterial->SetScalarParameterValue("AddFill", Post_Struct.bAddFill);
		// 描边宽度
		LastMaterial->SetScalarParameterValue("OutlineWidth", Post_Struct.Outline.OutlineWidth);
		// 描边颜色
		LastMaterial->SetVectorParameterValue("StrokeColor", Post_Struct.Outline.StrokeColor);
		// 描边强度
		LastMaterial->SetScalarParameterValue("StrokeStrength", Post_Struct.Outline.StrokeStrength);
		// 开启遮挡关系
		LastMaterial->SetScalarParameterValue("OcclusionRelation_Outline", Post_Struct.Outline.OcclusionRelation_Outline);
		// 遮挡关系
		LastMaterial->SetScalarParameterValue("DisplayOcclusion_Outline", Post_Struct.Outline.DisplayOcclusion_Outline);
		// 开启相交处的描边
		LastMaterial->SetScalarParameterValue("OpenIntersectingStroke", Post_Struct.Outline.OpenIntersectingStroke);
		// 填充的颜色
		LastMaterial->SetVectorParameterValue("FillColor", Post_Struct.Fill.FillColor);
		// 开启遮挡关系
		LastMaterial->SetScalarParameterValue("OcclusionRelation_Fill", Post_Struct.Fill.OcclusionRelation_Fill);
		// 遮挡关系
		LastMaterial->SetScalarParameterValue("DisplayOcclusion_Fill", Post_Struct.Fill.DisplayOcclusion_Fill);
		return LastMaterial;
	}
	return nullptr;
}

void APostStrokeAndFill::GetMaterials()
{
	if (PostSettings.IsValidIndex(0))
	{
		Materials.Reset();

		FName PackagePath;

		for (int i = 0; i <= PostSettings.Num() - 1; i++)
		{
			// 判断获取的对应文件
			if (PostSettings[i].bAnother)
			{
				PackagePath = TEXT("/PostProcessingMaterial/Post/PostMaterial/Sobel/");
			}
			else
			{
				PackagePath = TEXT("/PostProcessingMaterial/Post/PostMaterial/");
			}

			FString MaterialName = FString(TEXT("M_Outline"));

			// 添加获取的对应文件
			if (TArray<FAssetData> OutAssetData; AssetRegistry->GetAssetsByPath(PackagePath, OutAssetData))
			{
				TArray<UMaterialInterface*> TempMaterials;

				for (auto It = OutAssetData.CreateIterator(); It; ++It)
				{
					// 获取对应的资产
					if (It->GetClass() == UMaterial::StaticClass() && It->GetFullName().Contains(MaterialName))
					{
						TempMaterials.AddUnique(Cast<UMaterialInterface>(It->GetAsset()));
					}
				}

				if (TempMaterials.Num() >= PostSettings.Num())
				{
					for (int j = 0; j <= TempMaterials.Num() - 1; j++)
					{
						if (const int32 Last = FCString::Atoi(*Cast<UObject>(TempMaterials[j])->GetName().RightChop(9)); i == Last - 1)
						{
							Materials.AddUnique(TempMaterials[j]);
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		Materials.Reset();
	}
}
