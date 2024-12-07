// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 跳跃相关的拾取类
 */
UCLASS()
class TAKETHESHOT_API AJumpPickup : public APickup
{
	GENERATED_BODY()

public:

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
	// 跳跃速度
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	// 持续时间
	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};
