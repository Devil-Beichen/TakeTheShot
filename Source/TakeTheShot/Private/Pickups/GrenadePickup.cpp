// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/GrenadePickup.h"

#include "Character/BlasterCharacter.h"


void AGrenadePickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	bool bCanPickup = BlasterCharacter &&
		BlasterCharacter->GetCombat() &&
		BlasterCharacter->GetCombat()->GetGrenades() != BlasterCharacter->GetCombat()->GetMaxGrenades();
	if (bCanPickup)
	{
		BlasterCharacter->GetCombat()->PickupGrenade(Grenade);
		Destroy();
	}
}
