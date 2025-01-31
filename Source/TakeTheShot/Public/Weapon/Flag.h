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

private:
	// 旗子
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* FlagMesh;
};
