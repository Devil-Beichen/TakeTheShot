﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterCharacter;

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

	UPROPERTY()
	ABlasterCharacter* Character = nullptr;
};

// 服务器端回溯结果
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	// 是否命中
	UPROPERTY()
	bool bHitConfirmed = false;

	// 爆头
	UPROPERTY()
	bool bHeadShot = false;
};

// 服务器端霰弹枪回溯结果
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	// 击中头部
	TMap<ABlasterCharacter*, uint32> HeadShots;
	// 击中身体
	TMap<ABlasterCharacter*, uint32> BodyShots;
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

	/**
	 * 服务器端回溯函数，用于处理命中补偿
	 * @param HitCharacter	命中的角色
	 * @param TraceStart	命中的起始位置
	 * @param HitLocation	命中的位置
	 * @param HitTime		命中的时间
	 */
	FServerSideRewindResult ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/**
	* 确认命中
	* @param Package			需要插值的帧数据包
	* @param HitCharacter		命中的角色
	* @param TraceStart		命中的起始位置
	* @param HitLocation		命中的位置
	* @return 
	*/
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);


	/**
	 * 伤害请求（只会在服务器调用）
	 * @param HitCharacter	命中的角色
	 * @param TraceStart	命中的起始位置
	 * @param HitLocation	命中的位置
	 * @param HitTime		命中的时间
	 * @param DamageCauser	伤害的发起者
	 */
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		class AWeapon* DamageCauser
	);

protected:
	virtual void BeginPlay() override;

	// 存储帧数据
	void SaveFramePackage(FFramePackage& Package);

	/**
	 * 插值帧数据
	 * @param OlderFrame	旧帧
	 * @param YoungerFram	新帧
	 * @param HitTime		命中的时间
	 * @return 
	 */
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFram, float HitTime);

	/**
	 * 缓存角色碰撞框位置信息
	 * @param HitCharacter		命中的角色 
	 * @param OutFramePackage	缓存的帧数据包
	 */
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);

	/**
	 *	移动角色身上的碰撞盒到指定位置和旋转
	 * @param HitCharacter 被击中的角色，如果为nullptr则直接返回
	 * @param Package 包含了需要应用到碰撞盒上的位置、旋转和大小信息的帧包
	 */
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);

	/**
	 * 重置角色的碰撞盒位置和旋转
	* @param HitCharacter 被击中的角色，如果为nullptr则直接返回
	 * @param Package 包含了需要应用到碰撞盒上的位置、旋转和大小信息的帧包
	 */
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);

	// 启用或禁用角色的碰撞盒
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable);

	// 存储帧数据
	void SaveFramePackage();

	/**
	 * 获取需要检查的帧数据包
	 * @param HitCharacter	被击中的角色
	 * @param HitTime		命中的时间
	 * @return				命中的帧数据包
	 */
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	/**
	 * 霰弹枪
	 */
	// 服务器端霰弹枪回溯函数，用于处理命中补偿
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	// 确认霰弹枪命中
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& Framepackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

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