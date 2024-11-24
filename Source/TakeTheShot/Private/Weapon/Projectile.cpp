#include "Weapon/Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TakeTheShot.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 启用网络复制，使得在多机环境下可以正确同步状态
	bReplicates = true;

	// 创建一个默认的盒子碰撞体，用于检测和处理碰撞事件
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	// 将创建的碰撞盒设置为根组件，意味着它将成为其他所有组件的父级
	SetRootComponent(CollisionBox);
	// 设置碰撞体的对象类型为动态世界对象，影响它与其他对象的碰撞行为
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	// 启用碰撞体的碰撞检测和物理效果
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 对所有通道的默认响应设置为忽略，使得碰撞体不会受大多数物理效果的影响
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	// 对可见性通道的响应设置为阻挡，使得碰撞体可以遮挡视线和光线
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	// 对静态世界对象通道的响应设置为阻挡，使得碰撞体与静态环境的碰撞有效
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

// 启动定时器
void AProjectile::StartDestroyTime()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

// 删除
void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition,
			false
		);
	}
}

// 爆炸伤害 
void AProjectile::ExplodeDamage()
{
	if (const APawn* FiringPawn = GetInstigator(); FiringPawn && HasAuthority())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 世界上下文
				Damage, // 基础伤害
				10.f, // 最小伤害
				GetActorLocation(), // 中心起点
				DamageInnerRadius, // 伤害内半径
				DamageOuterRadius, // 伤害外半径
				1.f, // 衰减指数
				UDamageType::StaticClass(), // 伤害类型
				TArray<AActor*>(), // 忽略的Actor
				this, // 造成伤害的Actor
				FiringController // 伤害发起者的控制器
			);
		}
	}
}

// 检测碰撞事件
void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 生成效果
void AProjectile::GeneratingEffects()
{
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, GetActorLocation());
	}
}

// 生成特效
void AProjectile::Destroyed()
{
	GeneratingEffects();
	Super::Destroyed();
}
