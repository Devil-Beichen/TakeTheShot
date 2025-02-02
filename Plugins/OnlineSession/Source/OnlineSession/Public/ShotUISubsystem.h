// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShotUISubsystem.generated.h"

// 匹配类型枚举
UENUM(Blueprintable, BlueprintType)
enum EMatchType:uint8
{
	// 自由模式
	EM_FreeForAll UMETA(DisplayName = "FreeForAll"),
	// 团队模式
	EM_Teams UMETA(DisplayName = "Teams"),
	// 夺旗模式
	EM_CaptureTheFlag UMETA(DisplayName = "CaptureTheFlag"),
	// 默认
	EM_MAX UMETA(DisplayName = "DefaultMAX")
};


/**
 * 射击UI子系统
 */
UCLASS()
class ONLINESESSION_API UShotUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 是否创建子系统
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 初始化子系统
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 销毁子系统
	virtual void Deinitialize() override;

	// 匹配类型
	static EMatchType DesiredMatchType;

	// 设置匹配类型
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject"))
	static void SetDesiredMatchType(const UObject* WorldContextObject, EMatchType MatchType);
};
