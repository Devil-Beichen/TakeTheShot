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
	FVector Location = FVector();

	// 旋转
	UPROPERTY()
	FRotator Rotation = FRotator();

	// 大小
	UPROPERTY()
	FVector BoxExtent = FVector();
};

// 帧数据结构
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	// 时间
	UPROPERTY()
	float Time = 0.f;

	// 盒子信息
	TMap<FName, FBoxInFormation> HitBoxInfo = TMap<FName, FBoxInFormation>();
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
	ABlasterCharacter* Character = nullptr;
	// 玩家控制器
	UPROPERTY()
	class ABlasterPlayerController* Controller = nullptr;

	// 帧数据
	TDoubleLinkedList<FFramePackage> FrameHistory;

	// 添加帧数据包
	void AddFramePackage();

	// 获取帧数据包持续的时间长度
	float GetFrameTime() const;

	// 最大记录时间
	UPROPERTY(EditDefaultsOnly)
	float MaxRecordTime = 4.f;

public:
};
