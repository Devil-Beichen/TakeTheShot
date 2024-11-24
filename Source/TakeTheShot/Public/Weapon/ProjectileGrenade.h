// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 榴弹
 */
UCLASS()
class TAKETHESHOT_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileGrenade();

	// 销毁
	virtual void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/**
	 * 在反弹
	 * @param ImpactResult		碰撞结果	
	 * @param ImpactVelocity	碰撞速度
	 */
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	// 弹跳音效
	UPROPERTY(EditDefaultsOnly)
	USoundCue* BounceSound;
};
