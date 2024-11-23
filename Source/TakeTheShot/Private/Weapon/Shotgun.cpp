// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"

#include "Engine/SkeletalMeshSocket.h"


// Sets default values
AShotgun::AShotgun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	// 获取拥有此武器的Pawn
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	// 如果没有找到拥有此武器的Pawn，则不继续执行
	if (OwnerPawn == nullptr) return;

	// 获取控制拥有此武器的Pawn的控制器
	AController* InstigatorController = OwnerPawn->GetController();

	// 尝试获取武器网格上的"MuzzleFlash"插槽，并确保控制器已初始化
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		// 获取插槽的变换
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 获取射击起点的位置
		FVector Start = SocketTransform.GetLocation();
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			// 计算射击终点的位置，稍微远于目标以确保命中
			FVector End = TraceEndWithScatter(Start, HitTarget);
		}
	}
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
