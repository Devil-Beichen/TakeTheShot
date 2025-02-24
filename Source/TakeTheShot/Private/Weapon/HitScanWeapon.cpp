﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "BlasterComponents/LagCompensationComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Sound/SoundCue.h"

AHitScanWeapon::AHitScanWeapon()
{
	FireType = EFireType::EFT_HitScan;
	NumberOfPellets = 1;
	bUseServerSideRewind = true;
}

// 当击中目标时调用此函数来处理命中扫描武器的射击逻辑
void AHitScanWeapon::Fire(const TArray<FVector_NetQuantize>& HitTargets)
{
	Super::Fire(HitTargets);

	UWorld* World = GetWorld();
	if (World == nullptr)return;

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
		// 命中的角色
		TArray<ABlasterCharacter*> HitCharacters = TArray<ABlasterCharacter*>();
		// 创建一个哈希表，用于存储命中的目标和命中的次数
		TMap<ABlasterCharacter*, uint32> HitMap;
		// 创建一个哈希表，用于存储命中目标的头部和命中的次数
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
		// 循环遍历每个目标点
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			// 用于存储射击线迹的结果
			FHitResult FireHit;

			WeaponTraceHit(Start, HitTarget, FireHit);
			// 如果线迹测试击中了某个物体
			if (FireHit.bBlockingHit)
			{
				// 如果击中的演员是ABlasterCharacter类型
				if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
				{
					HitCharacters.AddUnique(BlasterCharacter);
					// 判断是否击中头部
					if (/*const bool bHeadShot = */FireHit.BoneName.ToString() == FString("head"))
					{
						// 如果是头部射击，增加HeadShotHitMap中对应角色的计数
						HeadShotHitMap.FindOrAdd(BlasterCharacter)++;
					}
					else
					{
						// 如果不是头部射击，增加HitMap中对应角色的计数
						HitMap.FindOrAdd(BlasterCharacter)++;
					}
				}
				// 如果有设置撞击粒子效果
				if (ImpactParticles)
				{
					// 在撞击点生成粒子效果
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}
		}

		// 创建一个哈希表，用于存储每个目标对应的伤害
		TMap<ABlasterCharacter*, float> DamageMap;
		if (!HitMap.IsEmpty()) // 如果有设置普通射击的哈希表
		{
			// 遍历哈希表，对每个目标应用伤害
			for (auto& HitPair : HitMap)
			{
				if (HitPair.Key)
				{
					DamageMap.FindOrAdd(HitPair.Key) += HitPair.Value * Damage;
				}
			}
		}
		if (!HeadShotHitMap.IsEmpty()) // 如果有设置头部射击的哈希表
		{
			// 遍历哈希表，对每个目标应用头部伤害
			for (auto& HeadShotHitPair : HeadShotHitMap)
			{
				if (HeadShotHitPair.Key)
				{
					DamageMap.FindOrAdd(HeadShotHitPair.Key) += HeadShotHitPair.Value * HeadShotDamage;
				}
			}
		}

		// 应用伤害给每个目标
		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

		for (auto& DamagePair : DamageMap)
		{
			// 如果拥有此武器的Pawn拥有 Authority 且 控制器有效 且 命中的目标有效
			if (HasAuthority() && InstigatorController && bCauseAuthDamage)
			{
				// 应用伤害给击中的对象
				UGameplayStatics::ApplyDamage(
					DamagePair.Key,
					DamagePair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}

		// 如果拥有此武器的Pawn拥有 Authority 且 控制器有效 且 命中的目标有效(启用延迟补偿)
		if (!HitCharacters.IsEmpty() && !HasAuthority() && bUseServerSideRewind)
		{
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;

			if (BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagComponent() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				BlasterOwnerCharacter->GetLagComponent()->ServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
				);
			}
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAttached(
				MuzzleFlash,
				GetWeaponMesh(),
				FName(),
				SocketTransform.GetLocation(),
				FRotator(),
				EAttachLocation::KeepWorldPosition
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				World,
				FireSound,
				Start
			);
		}
	}
}

// 武器命中效果
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector_NetQuantize& HitTarget, FHitResult& OutHit)
{
	if (UWorld* World = GetWorld())
	{
		// 计算射击终点的位置，稍微远于目标以确保命中
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		// 在起点和终点之间进行单次线迹测试，使用可见性通道
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECC_Visibility
		);
		FVector BeamEnd = End;
		// 如果线迹测试击中了某个物体
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		// 绘制命中点
		// DrawDebugSphere(World, BeamEnd, 15.f, 12, FColor(243, 156, 18), false, 3);

		if (BeamParticles)
		{
			// 在起点生成拖尾特效
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true))
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
