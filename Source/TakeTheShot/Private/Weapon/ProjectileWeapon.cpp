﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"


AProjectileWeapon::AProjectileWeapon()
{
	FireType = EFireType::EFT_Projectile;
	NumberOfPellets = 1;
}

// AProjectileWeapon::Fire - 向指定目标发射子弹
// 参数:
//   HitTarget - FVector类型，子弹命中目标的位置
void AProjectileWeapon::Fire(const TArray<FVector_NetQuantize>& HitTargets)
{
	Super::Fire(HitTargets);

	// 获取当前武器的所有者（持有者），并尝试将其转换为APawn类型
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	UWorld* World = GetWorld();

	// 尝试获取武器模型上的“MuzzleFlash”插槽
	if (MuzzleFlashSocket && World)
	{
		// 获取插槽的转换矩阵
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			// 计算从插槽位置到目标位置的向量
			const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
			// 根据向量计算目标的旋转角度
			const FRotator TargetRotation = ToTarget.Rotation();

			/*// 在本地和远程客户端上绘制调试信息
			if (!InstigatorPawn->HasAuthority())
			{
				DrawDebugLine(World, SocketTransform.GetLocation(), HitTarget, FColor::Green, true, 1.f, 1.f);
				DrawDebugSphere(World, SocketTransform.GetLocation(), 15.f, 12, FColor(0.f, 255.f, 0.f), true, 1.f, 1.f);
			}
			else
			{
				DrawDebugLine(World, SocketTransform.GetLocation(), HitTarget, FColor::Red, true, 1.f, 1.f);
				DrawDebugSphere(World, SocketTransform.GetLocation(), 15.f, 12, FColor(255.f, 0.f, 0.f), true, 1.f, 1.f);
			}*/

			// 设置生成参数
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			AProjectile* SpawnedProjectile = nullptr;

			InstigatorPawn = InstigatorPawn == nullptr ? Cast<APawn>(GetOwner()) : InstigatorPawn;

			if (!bUseServerSideRewind) // 不使用服务器回溯(延迟补偿)
			{
				if (!ProjectileClass) return;
				if (InstigatorPawn->HasAuthority())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					// 设置不用服务器回溯
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
				if (!ServerSideRewindProjectileClass) return;
				if (!InstigatorPawn->HasAuthority())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					// 设置不用服务器回溯
					SpawnedProjectile->bUseServerSideRewind = false;
					if (SpawnedProjectile->InitialSpeed < 20000) // 如果初始速度小于20000，则销毁子弹
					{
						SpawnedProjectile->ImpactParticles = nullptr;
						SpawnedProjectile->ImpactSound = nullptr;
						SpawnedProjectile->Destroy();
					}
				}
			}
			else // 使用服务器回溯(延迟补偿)
			{
				// 如果拥有者拥有服务器权限
				if (InstigatorPawn->HasAuthority())
				{
					if (InstigatorPawn->IsLocallyControlled()) // 拥有服务器权限是本地玩家
					{
						if (!ServerSideRewindProjectileClass) return;
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						// 设置使用服务器回溯
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->Damage = Damage;
						SpawnedProjectile->HeadShotDamage = HeadShotDamage;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
						UE_LOG(LogTemp, Warning, TEXT("在服务器生成！！！"));
						return;
					}
					else // 拥有服务器权限不是本地玩家
					{
						if (!ServerSideRewindProjectileClass)return;
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						// 设置需要服务器回溯
						SpawnedProjectile->bUseServerSideRewind = true;
						UE_LOG(LogTemp, Warning, TEXT("在客户端生成1！！！"));
					}
				}
				if (!InstigatorPawn->HasAuthority()) // 客户端
				{
					if (InstigatorPawn->IsLocallyControlled()) // 本地控制，生成非复制的子弹，使用服务器回溯
					{
						if (!ServerSideRewindProjectileClass) return;
						// 生成非复制的子弹，使用服务器回溯
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
						UE_LOG(LogTemp, Warning, TEXT("在客户端生成A！！！"));
					}
					else // 非本地控制，生成非复制的子弹，不使用服务器回溯
					{
						if (!ServerSideRewindProjectileClass) return;
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						SpawnedProjectile->bUseServerSideRewind = false;
						UE_LOG(LogTemp, Warning, TEXT("在客户端生成B！！！"));
						if (SpawnedProjectile->InitialSpeed < 20000) // 如果初始速度小于20000，则销毁子弹
						{
							SpawnedProjectile->ImpactParticles = nullptr;
							SpawnedProjectile->ImpactSound = nullptr;
							SpawnedProjectile->Destroy();
						}
					}
				}
			}
		}
	}
}

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
}
