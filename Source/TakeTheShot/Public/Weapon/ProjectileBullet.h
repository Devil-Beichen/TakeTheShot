// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 射击子弹
 */
UCLASS()
class TAKETHESHOT_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

	AProjectileBullet();

protected:
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
	virtual void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit) override;
};
