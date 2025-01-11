// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"

#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"
#include "BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// 检查BlasterCharacter是否已初始化，如果没有，则尝试将其设置为当前拥有的炮手角色
	if (BlasterCharacter == nullptr)
	{
		// 尝试将当前的炮手角色赋值给BlasterCharacter，如果不存在则不进行赋值
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	// 如果BlasterCharacter仍然为空，则直接返回，不进行后续计算
	if (BlasterCharacter == nullptr) return;

	// 获取BlasterCharacter的当前速度
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;

	// 计算处理后的速度的大小，并将其赋值给Speed变量，用于后续判断或使用
	Speed = Velocity.Size();

	// 检查BlasterCharacter是否处于坠落状态，即是否在空中
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	// 检查BlasterCharacter当前的加速度大小，如果大于0，则表示正在加速
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

	// 检查BlasterCharacter是否装备了武器
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();

	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();

	// 检查BlasterCharacter是否正在瞄准
	bAiming = BlasterCharacter->IsAiming();

	// 获取BlasterCharacter的转弯状态
	TurningInPlace = BlasterCharacter->GetTurningInPlace();

	// 检查BlasterCharacter是否需要旋转根骨骼
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();

	// 检查BlasterCharacter是否被淘汰
	bEliminate = BlasterCharacter->IsEliminate();

	// 获取角色的基础瞄准旋转
	const FRotator AinRotation = BlasterCharacter->GetBaseAimRotation();
	// 根据角色的移动方向创建旋转
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

	// 计算并获取移动旋转与目标旋转之间的规范化旋转差值
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AinRotation);

	// 使用平滑插值更新DeltaRotation，使其平滑地朝着计算出的DeltaRot旋转，插值速率为6.f
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);

	// 将旋转增量的偏航角赋值给YawOffset变量
	YawOffset = DeltaRotation.Yaw;

	// 在这一帧中更新角色的旋转状态
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	// 计算角色旋转的变化量
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	// 计算目标倾斜角度
	const float Target = Delta.Yaw / DeltaSeconds;
	// 平滑地 interpolate 到目标倾斜角度
	const float Interp = UKismetMathLibrary::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	// 限制倾斜角度在合理范围内
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	// 检查是否装备了武器，且武器对象和其网格模型以及BlasterCharacter（一种角色类型）的网格模型都有效
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// 获取左手法套的位置和旋转信息
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		// 准备存储转换后的左手位置和旋转变量
		FVector OutPosition;
		FRotator OutRotation;

		// 将左手的网格空间位置转换到骨架空间中
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		// 更新左手的位置和旋转
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			// 获取右手的网格空间位置和旋转信息
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation(BlasterCharacter->GetHitTarget(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - RightHandTransform.GetLocation()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotator, DeltaSeconds, 30.f);
		}
	}

	if (BlasterCharacter->GetDisableGameplay())
	{
		bUseFABRIK = false;
		bUseAimOffsets = false;
		bTransformRightHand = false;
	}
	else
	{
		bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
		bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() &&
			BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
			BlasterCharacter->bFinishedSwapping;
		if (bFABRIKOverride)
		{
			bUseFABRIK = !BlasterCharacter->IsLocallyReloading();
		}
		bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
		bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	}
}
