// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 护盾相关的拾取类
 */
UCLASS()
class TAKETHESHOT_API AShieldPickup : public APickup
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
	// 护盾补充量
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	// 护盾补充时间
	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 2.f;
};
