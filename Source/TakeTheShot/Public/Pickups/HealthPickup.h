﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 恢复健康值的拾取类
 */
UCLASS()
class TAKETHESHOT_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

protected:
	virtual void BeginPlay() override;

	/**
	* 当角色的碰撞球体与另一个演员的碰撞体发生重叠时，此函数将被调用。
	* 
	* @param OverlappedComponent 角色的碰撞球体组件，表示触发重叠的组件。
	* @param OtherActor 发生重叠的另一个演员。
	* @param OtherComp 另一个演员的碰撞组件。
	* @param OtherBodyIndex 另一个碰撞体的索引。
	* @param bFromSweep 如果重叠是由于扫动检测引起的，则此参数为true；否则为false。
	* @param SweepResult 扫动检测的结果，包含重叠的详细信息。
	*/
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	// 拾取后回复的生命值
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	// 回复时间
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

public:
};
