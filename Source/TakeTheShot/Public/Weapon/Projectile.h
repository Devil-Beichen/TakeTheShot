#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;

/**
 *	子弹基类
 */
UCLASS()
class TAKETHESHOT_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	// 设置销毁计时器启动
	void StartDestroyTime();

	// 销毁计时器结束
	void DestroyTimerFinished();

	// 创建拖尾特效
	void SpawnTrailSystem();

	// 爆炸伤害
	void ExplodeDamage();

	// 抛射体组件
	UPROPERTY(VisibleAnywhere, Category="Projectile")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// 抛洒物网格体
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	// 碰撞盒
	UPROPERTY(EditAnywhere, Category="Projectile")
	class UBoxComponent* CollisionBox;

	// 拖尾特效
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* TrailSystem;

	// 拖尾特效组件
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	/**
	 * 处理Actor的碰撞事件。
	 * 当这个Actor与其他Actor发生碰撞时，会调用此函数。
	 * 
	 * @param HitComponent 碰撞的组件，是调用此函数的Actor的一部分。
	 * @param OtherActor 发生碰撞的另一个Actor。
	 * @param OtherComp 另一个Actor参与碰撞的组件。
	 * @param NormalImpulse 碰撞产生的法向冲量，表示碰撞的力量和方向。
	 * @param Hit 碰撞结果对象，包含关于碰撞的详细信息，如碰撞点和碰撞正常向量。
	 */
	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	// 伤害值
	UPROPERTY(EditDefaultsOnly)
	float Damage = 10.f;

	// 伤害内半径
	UPROPERTY(EditDefaultsOnly)
	float DamageInnerRadius = 200.f;
	// 伤害外半径
	UPROPERTY(EditDefaultsOnly)
	float DamageOuterRadius = 500.f;

private:
	// 粒子特效
	UPROPERTY(EditAnywhere, Category="Projectile")
	UParticleSystem* Tracer;

	// 粒子特效组件
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	// 销毁计时器
	FTimerHandle DestroyTimer;

	// 销毁时间
	UPROPERTY(EditDefaultsOnly)
	float DestroyTime = 3.f;

	// 命中特效
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	UParticleSystem* ImpactParticles;

	// 命中声音
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	USoundCue* ImpactSound;

public:
	virtual void Tick(float DeltaTime) override;

	// 生成效果
	void GeneratingEffects();
};
