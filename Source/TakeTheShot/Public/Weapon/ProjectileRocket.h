// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 火箭子弹
 */
UCLASS()
class TAKETHESHOT_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

	virtual void Destroyed() override;

protected:
	// 命中
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	// 开始
	virtual void BeginPlay() override;

	// 循环音效
	UPROPERTY(EditDefaultsOnly)
	USoundCue* ProjectileLoop;

	// 循环音效组件
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	// 循环音效衰减
	UPROPERTY(EditDefaultsOnly)
	USoundAttenuation* LoopingSoundAttenuation;

	// 火箭移动组件
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:
};
