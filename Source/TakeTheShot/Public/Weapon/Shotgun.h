﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 霰弹枪
 */
UCLASS()
class TAKETHESHOT_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShotgun();

	virtual void Fire(const FVector& HitTarget) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 霰弹枪弹丸数量
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	uint32 NumberOfPellets = 10;
};