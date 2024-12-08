// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

/**
 * 拾取刷新点
 */
UCLASS()
class TAKETHESHOT_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawnPoint();

protected:
	virtual void BeginPlay() override;

	// 拾取类型
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	// 生成的拾取类
	UPROPERTY()
	APickup* SpawndPickup = nullptr;

	// 生成拾取类
	void SpawnPickup();

	// 生成拾取类计时器结束
	void SpawnPickupTimerFinished();

	/**
	 * 拾取类销毁后，重新生成拾取类
	 * @param DestroyedActor 销毁的 Actor
	 */
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:
	// 生成拾取类计时器
	FTimerHandle SpawnPickupTime;

	// 生成拾取类时间
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin = 20.f;
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax = 30.f;

public:
};
