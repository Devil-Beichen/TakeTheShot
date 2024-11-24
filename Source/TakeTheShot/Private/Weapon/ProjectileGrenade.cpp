// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	// 创建一个默认的移动组件，用于控制物体的移动和方向
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

	// 启用移动组件的旋转跟随速度，使得物体的朝向与移动方向一致
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// 将移动组件设置为同步，以实现多客户端同步
	ProjectileMovementComponent->SetIsReplicated(true);
	// 启用反弹，以实现弹跳效果
	ProjectileMovementComponent->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	StartDestroyTime(); // 启动销毁计时器
	SpawnTrailSystem(); // 创建轨迹系统

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

// 在反弹
void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// 播放声音
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	}
}

// 销毁
void AProjectileGrenade::Destroyed()
{
	ExplodeDamage(); // 触发爆炸伤害
	Super::Destroyed();
}
