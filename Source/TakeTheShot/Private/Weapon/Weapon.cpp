// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"

#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 创建默认子对象USkeletalMeshComponent，用于表示武器的网格组件
	// 使用文本"WeaponMesh"作为名称，以便在编辑器中识别
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));

	// 设置根组件为WeaponMesh
	// 这一步是将WeaponMesh作为刚体的主要组件，用于物理交互
	SetRootComponent(WeaponMesh);

	// 设置WeaponMesh的碰撞响应为阻挡所有通道
	// 使得WeaponMesh能够阻止其他物体穿过它
	WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Block);

	// 设置WeaponMesh对Pawn通道的碰撞响应为忽略
	// 这样武器不会阻止角色的移动和交互
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// 禁止WeaponMesh的碰撞检测
	// 即使其他物体与WeaponMesh相交，也不会产生碰撞效果
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建默认子对象USphereComponent，用于表示武器的影响区域
	// 使用文本"AreaSphere"作为名称，以便在编辑器中识别
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));

	// 将AreaSphere设置为根组件的附件
	// 与WeaponMesh类似，AreaSphere也会随着根组件一起移动和旋转
	AreaSphere->SetupAttachment(RootComponent);

	// 设置AreaSphere对所有通道的碰撞响应为忽略
	// 这样AreaSphere不会影响其他物体的运动和交互
	AreaSphere->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);

	// 禁止AreaSphere的碰撞检测
	// 与WeaponMesh相同，AreaSphere不会产生碰撞效果
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建默认子对象UWidgetComponent，用于表示武器的武器拾取组件
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	// 附加到根组件上
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * 如果当前对象具有权限，则设置AreaSphere的碰撞属性。
	 * 这里的目的是让AreaSphere对Pawn类型的对象只进行碰撞检测，而不进行碰撞响应，
	 * 也就是说，Pawn可以穿过AreaSphere，但AreaSphere不会对Pawn产生碰撞效果。
	 */
	if (HasAuthority())
	{
		// 启用碰撞检测和物理碰撞
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 对于Pawn类型的对象，设置碰撞响应为重叠，即不产生碰撞效果
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		// 添加一个事件监听器，当AreaSphere与Pawn类型对象发生重叠时，调用OnSphereOverlap函数
		// OnSphereOverlap函数用于处理武器的拾取逻辑
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);

		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	ShowPickupWidget(false);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::SetWeaponState(const EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		// 隐藏武器的拾取提示，因为已经装备完毕
		ShowPickupWidget(false);
	// 将武器的碰撞检查关闭（只在服务器调用）
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

// 当武器状态发生变化时调用的函数
void AWeapon::OnRep_WeaponState()
{
	// 根据武器状态执行不同的操作
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped: // 当武器状态为装备中
		// 隐藏拾取小部件，因为武器已经被装备
		ShowPickupWidget(false);
		break;
	}
}


void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
	// 根据传入的参数bShowWidget来设置物品获取提示的可见性和武器模型的自定义深度渲染状态
	// 这里主要处理的是UI提示和渲染效果的开关，以便在不同的游戏情境下提供更合适的视觉反馈
	if (PickupWidget)
	{
		// 设置物品获取提示的可见性
		PickupWidget->SetVisibility(bShowWidget);
		// 根据物品获取提示的可见性状态，设置武器模型的自定义深度渲染
		WeaponMesh->SetRenderCustomDepth(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget) const
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor); PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor); PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}
