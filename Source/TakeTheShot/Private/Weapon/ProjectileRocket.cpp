﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"


AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (const APawn* FiringPawn = GetInstigator())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 世界上下文
				Damage, // 基础伤害
				10.f, // 最小伤害
				GetActorLocation(), // 中心起点
				200.f, // 伤害内半径
				500.f, // 伤害外半径
				1.f, // 衰减指数
				UDamageType::StaticClass(), // 伤害类型
				TArray<AActor*>(), // 忽略的Actor
				this, // 造成伤害的Actor
				FiringController // 伤害发起者的控制器
			);
		}
	}
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}