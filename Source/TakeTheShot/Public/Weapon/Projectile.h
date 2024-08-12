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

private:
	// 碰撞盒
	UPROPERTY(EditAnywhere, Category="Projectile")
	class UBoxComponent* CollisionBox;

	// 抛射体组件
	UPROPERTY(VisibleAnywhere, Category="Projectile")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// 粒子特效
	UPROPERTY(EditAnywhere, Category="Projectile")
	UParticleSystem* Tracer;

	// 粒子特效组件
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	// 命中特效
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	UParticleSystem* ImpactParticles;

	// 命中声音
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	USoundCue* ImpactSound;

public:
	virtual void Tick(float DeltaTime) override;
};
