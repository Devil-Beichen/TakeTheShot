// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"


AProjectileWeapon::AProjectileWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

// AProjectileWeapon::Fire - 向指定目标发射子弹
// 参数:
//   HitTarget - FVector类型，子弹命中目标的位置
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	// 调用基类的Fire方法，执行一些基础的发射动作
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;

	// 获取当前武器的所有者（持有者），并尝试将其转换为APawn类型
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// 尝试获取武器模型上的“MuzzleFlash”插槽
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash")))
	{
		// 获取插槽的转换矩阵
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 计算从插槽位置到目标位置的向量
		const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// 根据向量计算目标的旋转角度
		const FRotator TargetRotation = ToTarget.Rotation();

		// 检查是否已设定ProjectileClass且武器的所有者有效
		if (ProjectileClass && InstigatorPawn)
		{
			// 设置生成参数
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			// 获取当前世界
			if (UWorld* World = GetWorld())
			{
				// 在世界中生成一个子弹Actor
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
				);
			}
		}
	}
}


void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
}
