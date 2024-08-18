// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

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
