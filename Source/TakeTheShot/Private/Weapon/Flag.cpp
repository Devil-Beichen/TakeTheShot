// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Flag.h"

#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AFlag::AFlag()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponType = EWeaponType::EWT_Flag;

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));

	SetRootComponent(FlagMesh);

	GetAreaSphere()->SetupAttachment(FlagMesh);

	GetPickupWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

// 丢弃武器
void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	// 将武器从父组件中分离（即从装备者身上分离）
	FlagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}

// 装备武器
void AFlag::OnEquipped()
{
	// 隐藏武器的拾取提示，因为已经装备完毕
	ShowPickupWidget(false);
	// 关闭碰撞检测
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 关闭物理模拟
	FlagMesh->SetSimulatePhysics(false);
	// 关闭物理
	FlagMesh->SetEnableGravity(false);
	// 设置碰撞响应为重叠
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
}

// 丢弃武器
void AFlag::OnDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_INTERITEM);
	FlagMesh->MarkRenderStateDirty();
}

// 重置旗子
void AFlag::ResetFlag()
{
	if (ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner()))
	{
		FlagBearer->SetbHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->bUseControllerRotationYaw = true;
		FlagBearer->GetCharacterMovement()->bOrientRotationToMovement = false;
		FlagBearer->UnCrouch();
	}
	// 如果不是服务器，则返回
	if (!HasAuthority())return;

	SetWeaponState(EWeaponState::EWS_Initial);

	// 启用碰撞检测和物理碰撞
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 对于Pawn类型的对象，设置碰撞响应为重叠，即不产生碰撞效果
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// 将武器从父组件中分离（即从装备者身上分离）
	FlagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

	SetActorTransform(InitialTransform);
}
