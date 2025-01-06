// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"

AShotgun::AShotgun()
{
	FireType = EFireType::EFT_Shotgun;
	NumberOfPellets = 12;
}

// Called when the game starts or when spawned
void AShotgun::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShotgun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
