// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpeedPickup.h"

#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"


ASpeedPickup::ASpeedPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ASpeedPickup::BeginPlay()
{
	Super::BeginPlay();
}

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->GetBuff())
	{
		BlasterCharacter->GetBuff()->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		Destroy();
	}
}

void ASpeedPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
