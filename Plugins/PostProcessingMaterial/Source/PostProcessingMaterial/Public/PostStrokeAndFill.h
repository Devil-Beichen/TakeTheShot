// 未经本人允许禁止私自传播

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Scene.h"
#include "PostStrokeAndFill.generated.h"

/* 描边的结构体 */
USTRUCT(BlueprintType)
struct FOutline_Struct
{
	GENERATED_BODY()

public:
	/** 描边宽度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="描边宽度"), Category="描边")
	float OutlineWidth = 2.f;

	/** 描边颜色 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="描边颜色"), Category="描边")
	FLinearColor StrokeColor = FLinearColor(1, 0, 0, 1);

	/** 描边强度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="描边强度"), Category="描边")
	float StrokeStrength = 1.f;

	/** 开启遮挡关系 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="开启遮挡关系"), Category="描边")
	uint8 OcclusionRelation_Outline :1;

	/** 遮挡关系 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="遮挡关系", EditCondition = "OcclusionRelation_Outline == true", EditConditionHides), Category="描边")
	uint8 DisplayOcclusion_Outline :1;

	/** 开启相交描边 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="开启相交描边", EditCondition = "OcclusionRelation_Outline == true", EditConditionHides), Category="描边")
	uint8 OpenIntersectingStroke :1;

	FOutline_Struct()
	{
		OcclusionRelation_Outline = false;
		DisplayOcclusion_Outline = false;
		OpenIntersectingStroke = false;
	}
};

/* 填充的结构体 */
USTRUCT(BlueprintType)
struct FFill_Struct
{
	GENERATED_BODY()

public:
	/** 填充颜色 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="填充颜色"), Category="填充")
	FLinearColor FillColor = FLinearColor(1, 1, 1, 1);

	/** 开启遮挡关系 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="开启遮挡关系"), Category="填充")
	uint8 OcclusionRelation_Fill :1;

	/** 遮挡关系 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="遮挡关系", EditCondition = "OcclusionRelation_Fill == true", EditConditionHides), Category="填充")
	uint8 DisplayOcclusion_Fill :1;

	FFill_Struct()
	{
		OcclusionRelation_Fill = false;
		DisplayOcclusion_Fill = false;
	}
};

/** 后期设置结构体 */
USTRUCT(BlueprintType)
struct FPost_Struct
{
	GENERATED_BODY()

public:
	/** 启用材质 */
	UPROPERTY(EditAnywhere, meta=(DisplayName="启用材质", InlineEditConditionToggle = true), Category="默认")
	uint8 bEnableMaterial :1;

	/** 自定义模具值 */
	UPROPERTY(EditAnywhere, meta=(DisplayName="自定义模具值", EditCondition = "bEnableMaterial", UIMin="1", UIMax="255", ClampMin ="1", ClampMax = "255"), Category="默认")
	uint8 CustomDepthStencilValue = 1;

	// 另外一个描边材质
	UPROPERTY(EditAnywhere, meta=(DisplayName = "另一个描边材质"), Category="默认")
	uint8 bAnother :1;

	/** 开启描边 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="开启描边"), Category="默认")
	uint8 bOutline:1;

	// 外描边
	UPROPERTY(EditAnywhere, meta=(DisplayName = "外描边", EditCondition = "bAnother == true && bOutline == true", EditConditionHides), Category="描边")
	uint8 bOuterStrokes :1;

	// 分辨率
	UPROPERTY(EditAnywhere, meta=(DisplayName = "分辨率", EditCondition = "bAnother == true && bOutline == true", EditConditionHides), Category="描边")
	FVector2D Resolution = FVector2D(2560.f, 1440.f);

	/* 描边设置 */
	UPROPERTY(EditAnywhere, DisplayName="描边", meta=(EditCondition = "bOutline == true", EditConditionHides), Category="默认")
	FOutline_Struct Outline;

	/** 添加填充 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="添加填充"), Category="默认")
	uint8 bAddFill :1;

	/* 填充设置 */
	UPROPERTY(EditAnywhere, DisplayName="填充", meta=(EditCondition = "bAddFill == true", EditConditionHides), Category="默认")
	FFill_Struct Fill;

	FPost_Struct()
	{
		bEnableMaterial = true;
		bAnother = false;
		bOutline = true;
		bOuterStrokes = false;
		bAddFill = false;
	}
};

/* 后期材质描边和填充
 *
 */
UCLASS()
class POSTPROCESSINGMATERIAL_API APostStrokeAndFill : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APostStrokeAndFill();

	// 添加自定义后期材质
	UPROPERTY(EditAnywhere, meta=(DisplayName = "自定义后期材质"), Category="默认")
	FWeightedBlendables CustomPostMaterial;

	// 后期参数设置
	UPROPERTY(EditAnywhere, meta=(DisplayName = "后期材质参数设置"), Category="默认")
	TArray<FPost_Struct> PostSettings;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	//基础-在Editor中修改属性-覆盖
	//virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	/* 添加后期材质 */
	UFUNCTION()
	void AddPostMaterial();

	/** 设置后期材质属性
	 * @brief 
	 * @param Material			创建的材质
	 * @param Post_Struct		后期属性
	 * @return 
	 */
	UFUNCTION()
	UMaterialInterface* SetPostMaterialProperties(UMaterialInterface* Material, const FPost_Struct& Post_Struct);

public:
	/** 后期盒子 */
	UPROPERTY(VisibleAnywhere, Category="默认")
	TObjectPtr<class UPostProcessComponent> PostProcess;

	/* 后期 */
	UPROPERTY()
	FWeightedBlendables PostProcessMaterial;

	/* 材质 */
	UPROPERTY()
	TArray<UMaterialInterface*> Materials;

	// 获取内容浏览器的材质并赋值到数组
	void GetMaterials();

	// 资产注册
	const class IAssetRegistry* AssetRegistry;
};
