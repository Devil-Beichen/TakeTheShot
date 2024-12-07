// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/HealthPickup.h"

#include "Character/BlasterCharacter.h"
#include "BlasterComponents/BuffComponent.h"


AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
}

// 重叠事件
void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->GetBuff())
	{
		BlasterCharacter->GetBuff()->Heal(HealAmount, HealingTime);
		Destroy();
	}
}
