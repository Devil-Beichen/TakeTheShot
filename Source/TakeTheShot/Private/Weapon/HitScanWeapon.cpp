// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

// 当击中目标时调用此函数来处理命中扫描武器的射击逻辑
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	// 调用父类的Fire函数以执行任何预射击逻辑
	Super::Fire(HitTarget);

	// 获取拥有此武器的Pawn
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	// 如果没有找到拥有此武器的Pawn，则不继续执行
	if (OwnerPawn == nullptr) return;

	// 获取控制拥有此武器的Pawn的控制器
	AController* InstigatorController = OwnerPawn->GetController();


	// 尝试获取武器网格上的"MuzzleFlash"插槽，并确保控制器已初始化
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"); InstigatorController)
	{
		// 获取插槽的变换
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 获取射击起点的位置
		FVector Start = SocketTransform.GetLocation();
		// 计算射击终点的位置，稍微远于目标以确保命中
		FVector End = Start + (HitTarget - Start) * 1.25f;

		// 用于存储射击线迹的结果
		FHitResult FireHit;
		// 如果世界对象存在
		if (UWorld* World = GetWorld())
		{
			// 在起点和终点之间进行单次线迹测试，使用可见性通道
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);
			// 如果线迹测试击中了某个对象
			if (FireHit.bBlockingHit && FireHit.GetActor())
			{
				// 如果当前对象有权限执行伤害操作
				if (HasAuthority())
				{
					UE_LOG(LogTemp, Log, TEXT("击中了：%s - %f点伤害"), *FireHit.GetActor()->GetName(), Damage)
					// 应用伤害给击中的对象
					UGameplayStatics::ApplyDamage(
						FireHit.GetActor(),
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
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
			}
		}
	}
}
