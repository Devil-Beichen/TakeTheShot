// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Flag.h"

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
