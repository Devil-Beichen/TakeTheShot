#include "Weapon/Projectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
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

	// 创建一个默认的移动组件，用于控制物体的移动和方向
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

	// 启用移动组件的旋转跟随速度，使得物体的朝向与移动方向一致
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
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

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::Destroyed()
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, GetActorLocation());
	}
	Super::Destroyed();
}
