﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

/**
 *	子弹基类
 */
UCLASS()
class TAKETHESHOT_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

private:
	// 碰撞盒
	UPROPERTY(EditAnywhere, Category="Projectile")
	class UBoxComponent* CollisionBox;

public:
	virtual void Tick(float DeltaTime) override;
};