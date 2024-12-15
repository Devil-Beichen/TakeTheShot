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
	AHitScanWeapon();
	// 开火
	virtual void Fire(const FVector& HitTarget) override;

protected:
	/**
	 *	武器命中效果
	 * @param TraceStart   追踪起点
	 * @param HitTarget    命中点
	 * @param OutHit       命中信息
	 */
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	// 伤害
	UPROPERTY(EditDefaultsOnly)
	float Damage = 20.f;

	// 命中特效
	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* ImpactParticles;

	// 拖尾特效
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BeamParticles;

	// 枪口特效
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFlash;

	// 枪口音效
	UPROPERTY(EditDefaultsOnly)
	USoundCue* FireSound;

	// 命中音效
	UPROPERTY(EditDefaultsOnly)
	USoundCue* HitSound;

private:

};
