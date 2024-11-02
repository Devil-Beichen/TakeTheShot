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

	// 销毁计时器结束
	UFUNCTION()
	void DestroyTimerFinished();

	// 拖尾特效
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* TrailSystem;

	// 拖尾特效组件
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	// 循环音效
	UPROPERTY(EditDefaultsOnly)
	USoundCue* ProjectileLoop;

	// 循环音效组件
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	// 循环音效衰减
	UPROPERTY(EditDefaultsOnly)
	USoundAttenuation* LoopingSoundAttenuation;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	// 销毁计时器
	FTimerHandle DestroyTimer;

	// 销毁时间
	UPROPERTY(EditDefaultsOnly)
	float DestroyTime = 3.f;
};
