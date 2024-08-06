#include "Weapon/Projectile.h"

#include "Components/BoxComponent.h"


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
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
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
