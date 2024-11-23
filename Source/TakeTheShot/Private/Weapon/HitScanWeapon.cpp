// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

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
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
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
			FVector BeamEnd = End;
			// 如果线迹测试击中了某个物体
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				// 如果拥有此武器的Pawn拥有 Authority 且 控制器有效 且 命中的目标有效
				if (HasAuthority() && InstigatorController && FireHit.GetActor())
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
						BeamEnd,
						FireHit.ImpactNormal.Rotation()
					);
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						World,
						HitSound,
						BeamEnd
					);
				}
				if (BeamParticles)
				{
					// 在起点生成拖尾特效
					if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform))
					{
						Beam->SetVectorParameter(FName("Target"), BeamEnd);
					}
				}
			}
			if (MuzzleFlash)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					World,
					MuzzleFlash,
					SocketTransform
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
}

// 散弹枪的命中扫描武器的 Fire 函数
FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	// 获取从发射点到命中目标的向量
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();

	// 球的中心点
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToShere;
	FVector Randvec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + Randvec;
	FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true);

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
