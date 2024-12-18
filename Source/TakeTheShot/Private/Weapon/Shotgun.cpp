// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"

#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"


AShotgun::AShotgun()
{
	FireType = EFireType::EFT_Shotgun;
	NumberOfPellets = 12;
}

void AShotgun::Fire(const TArray<FVector_NetQuantize>& HitTargets)
{
	// 调用父类函数
	AWeapon::Fire(HitTargets);

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

		// 创建一个哈希表，用于存储命中的目标和命中的次数
		TMap<ABlasterCharacter*, uint32> HitMap;

		// 遍历所有命中的目标
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			// 追武器命中效果
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());

			// 如果拥有此武器的Pawn拥有 Authority 且 控制器有效 且 命中的目标有效
			if (HasAuthority() && InstigatorController && BlasterCharacter)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}
			if (ImpactParticles) // 如果有设置撞击粒子效果
			{
				// 在撞击点生成粒子效果
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound) // 如果有设置撞击声音
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					HitSound,
					FireHit.ImpactPoint,
					0.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
		}

		// 遍历哈希表，对每个目标应用伤害
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController && HasAuthority()) // 如果拥有此武器的Pawn拥有 Authority 且 控制器有效 且 命中的目标有效
			{
				// 应用伤害给击中的对象
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
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
