// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 旗子
 */
UCLASS()
class TAKETHESHOT_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

	// 重写丢弃武器
	virtual void Dropped() override;

protected:
	// 重写装备武器
	virtual void OnEquipped() override;

	// 重写丢弃武器
	virtual void OnDropped() override;

private:
	// 旗子
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* FlagMesh;
};
