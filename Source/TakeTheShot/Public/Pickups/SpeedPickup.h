// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 速度相关的拾取类
 */
UCLASS()
class TAKETHESHOT_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

public:
	ASpeedPickup();

protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	// 基础速度 buff
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	// 蹲下速度 buff
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;

	// 速度 buff 时间
	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;
};
