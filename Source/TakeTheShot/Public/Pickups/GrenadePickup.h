// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "GrenadePickup.generated.h"

/**
 * 手雷拾取类
 */
UCLASS()
class TAKETHESHOT_API AGrenadePickup : public APickup
{
	GENERATED_BODY()

public:

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

public:
	// 手雷数量
	UPROPERTY(EditAnywhere)
	int32 Grenade = 4;
};
