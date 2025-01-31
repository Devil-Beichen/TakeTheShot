// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Flag.h"

#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"


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

	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
