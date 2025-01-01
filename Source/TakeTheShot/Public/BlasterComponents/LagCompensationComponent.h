// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

// 盒子数据结构
USTRUCT(BlueprintType)
struct FBoxInFormation
{
	GENERATED_BODY()

	// 位置
	UPROPERTY()
	FVector Location;

	// 旋转
	UPROPERTY()
	FRotator Rotation;

	// 大小
	UPROPERTY()
	FVector BoxExtent;
};

// 帧数据结构
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	// 时间
	UPROPERTY()
	float Time;

	// 盒子信息
	TMap<FName, FBoxInFormation> HitBoxInfo;
};

/**
 * 延迟补偿组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TAKETHESHOT_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 显示帧数据包
	 * @param Package 需要显示的帧数据包
	 * @param Color 颜色
	 */
	void ShowFramePackage(const FFramePackage& Package, FColor Color);

protected:
	virtual void BeginPlay() override;

	// 存储帧数据
	void SaveFramePackage(FFramePackage& Package);

private:
	// 角色
	UPROPERTY()
	ABlasterCharacter* Character;
	// 玩家控制器
	UPROPERTY()
	class ABlasterPlayerController* Controller;

public:
};
