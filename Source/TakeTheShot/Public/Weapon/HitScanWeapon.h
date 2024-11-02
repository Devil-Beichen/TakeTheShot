// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 打射线的武器
 */
UCLASS()
class TAKETHESHOT_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	// 开火
	virtual void Fire(const FVector& HitTarget) override;

	// 伤害
	UPROPERTY(EditDefaultsOnly)
	float Damage = 20.f;

	// 命中特效
	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* ImpactParticles;
};
