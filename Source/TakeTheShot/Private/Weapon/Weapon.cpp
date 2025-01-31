// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"

#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Weapon/Casing.h"


AWeapon::AWeapon()
{
	bReplicates = true;

	// 设置网格组件的复制模式为 replication
	SetReplicatingMovement(true);

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
	// 设置默认的自定义通道
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_INTERITEM);
	// 标记渲染状态Dirty
	WeaponMesh->MarkRenderStateDirty();
}

// 启用或禁用自定义深度
void AWeapon::EnableCustomDepth(const bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Ping值过高
void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * 如果当前对象具有权限，则设置AreaSphere的碰撞属性。
	 * 这里的目的是让AreaSphere对Pawn类型的对象只进行碰撞检测，而不进行碰撞响应，
	 * 也就是说，Pawn可以穿过AreaSphere，但AreaSphere不会对Pawn产生碰撞效果。
	 */

	// 启用碰撞检测和物理碰撞
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 对于Pawn类型的对象，设置碰撞响应为重叠，即不产生碰撞效果
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// 添加一个事件监听器，当AreaSphere与Pawn类型对象发生重叠时，调用OnSphereOverlap函数
	// OnSphereOverlap函数用于处理武器的拾取逻辑
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);

	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	ShowPickupWidget(false);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	// 仅向拥有的角色同步
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

// 当角色的拥有者发生变化时回调函数
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

// 消耗子弹
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

// 添加子弹
void AWeapon::AddAmmo(int AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

// 客户端更新弹药数量
void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority())return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

// 客户端添加子弹
void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority())return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

// 显示或隐藏物品获取提示
void AWeapon::ShowPickupWidget(const bool bShowWidget)
{
	// 根据传入的参数bShowWidget来设置物品获取提示的可见性和武器模型的自定义深度渲染状态
	// 这里主要处理的是UI提示和渲染效果的开关，以便在不同的游戏情境下提供更合适的视觉反馈
	if (PickupWidget)
	{
		// 设置物品获取提示的可见性
		PickupWidget->SetVisibility(bShowWidget);
		// 根据物品获取提示的可见性状态，设置武器模型的自定义深度渲染
		EnableCustomDepth(bShowWidget);
	}
}

// 设置武器的状态
void AWeapon::SetWeaponState(const EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

// 当武器状态发生变化时调用的函数
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

// 设置武器的状态
void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	case EWeaponState::EWS_MAX:
		break;
	}
}

// 当武器被装备时调用的函数
void AWeapon::OnEquipped()
{
	// 隐藏武器的拾取提示，因为已经装备完毕
	ShowPickupWidget(false);
	// 关闭碰撞检测
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 关闭物理模拟
	WeaponMesh->SetSimulatePhysics(false);
	// 关闭物理
	WeaponMesh->SetEnableGravity(false);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 如果武器类型为冲锋枪，则开启物理模拟
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	}
	AddPingDelegate();
}

// 装备副武器
void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	if (HasAuthority())
	{
		// 将武器的碰撞检查关闭（只在服务器调用）
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	}

	RemovePingDelegate();
}

// 武器丢弃
void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_INTERITEM);

	RemovePingDelegate();
}

// 丢弃武器
void AWeapon::Dropped()
{
	// 将武器从父组件中分离（即从装备者身上分离）
	WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetWeaponState(EWeaponState::EWS_Dropped);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

// 添加Ping的代理
void AWeapon::AddPingDelegate()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

// 移除延迟检测的委托
void AWeapon::RemovePingDelegate()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

// 射击
void AWeapon::Fire(const TArray<FVector_NetQuantize>& HitTargets)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		// 尝试获取武器模型上的“AmmoEject”插槽
		if (const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject")))
		{
			// 获取插槽的转换矩阵
			const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());

			// 获取当前世界
			if (UWorld* World = GetWorld())
			{
				// 在世界中生成一个子弹壳Actor
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}

	SpendRound();
}

// 随机散射命中点
void AWeapon::TraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	// 尝试获取武器网格上的"MuzzleFlash"插槽，并确保控制器已初始化
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	// 获取插槽的变换
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	// 获取射击起点的位置
	const FVector Start = SocketTransform.GetLocation();

	// 获取从发射点到命中目标的向量
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	// 球的中心点
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToShere;

	// 生成随机的球体半径
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector Randvec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + Randvec;
		FVector ToEndLoc = EndLoc - Start;
		ToEndLoc = Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
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
