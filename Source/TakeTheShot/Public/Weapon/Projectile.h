#pragma once

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

	// 抛射体组件
	UPROPERTY(VisibleAnywhere, Category="Projectile")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// 粒子特效
	UPROPERTY(EditAnywhere, Category="Projectile")
	class UParticleSystem* Tracer;

	// 粒子特效组件
	class UParticleSystemComponent* TracerComponent;

public:
	virtual void Tick(float DeltaTime) override;
};
