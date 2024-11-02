// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	// 创建一个默认的移动组件，用于控制物体的移动和方向
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

	// 启用移动组件的旋转跟随速度，使得物体的朝向与移动方向一致
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// 将移动组件设置为同步，以实现多客户端同步
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 检查并获取OwnerCharacter，仅当OwnerCharacter是ACharacter的实例时执行后续操作
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		// 检查并获取OwnerController，确保OwnerCharacter有控制器
		if (AController* OwnerController = OwnerCharacter->Controller)
		{
			// 对OtherActor造成伤害，伤害值为Damage，施加伤害的控制器为OwnerController，使用UDamageType类型的静态类
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}

	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
