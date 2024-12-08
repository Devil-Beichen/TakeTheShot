// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupSpawnPoint.h"

#include "Pickups/Pickup.h"


APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	StartSpawnPickupTimer((AActor*)nullptr);
}

// 生成拾取类
void APickupSpawnPoint::SpawnPickup()
{
	if (!HasAuthority()) return;
	int32 NumpickupClasses = PickupClasses.Num();
	if (NumpickupClasses > 0)
	{
		FActorSpawnParameters ActorSpawnParameters;
		// 忽略碰撞
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		int32 Selection = FMath::RandRange(0, NumpickupClasses - 1);
		SpawndPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform(), ActorSpawnParameters);
		if (SpawndPickup)
		{
			SpawndPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

// 计时器完毕开始生成
void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	SpawnPickup();
}

// 开始启动计时器生成
void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	// 生成的时间
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickupTime,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime);
}
