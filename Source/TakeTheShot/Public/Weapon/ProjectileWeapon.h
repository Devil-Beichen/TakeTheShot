﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

/**
 *	发射子弹的武器
 */
UCLASS()
class TAKETHESHOT_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AProjectileWeapon();

	virtual void Fire(const TArray<FVector_NetQuantize>& HitTargets) override;;

protected:
	virtual void BeginPlay() override;

private:
	// 子弹类
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	// 服务器回溯的子弹类
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
