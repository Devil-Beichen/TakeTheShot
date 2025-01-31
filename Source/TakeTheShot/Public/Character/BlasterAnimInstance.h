// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**	爆破角色的基类
 * 
 */
UCLASS()
class TAKETHESHOT_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/**
	 * 初始化动画的本地实现。
	 * 虚拟函数，用于在对象初始化时设置动画状态。此函数被设计为被子类重写，
	 * 以实现特定于子类的动画初始化逻辑。
	 * @override 说明此函数是对其基类函数的重写。
	 */
	virtual void NativeInitializeAnimation() override;

	/**
	 * 更新动画的本地实现。
	 * 虚拟函数，用于在每个帧更新对象的动画状态。此函数被设计为被子类重写，
	 * 以实现特定于子类的动画更新逻辑。
	 * @param DeltaSeconds 动画更新的时间间隔，以秒为单位。此参数用于计算动画的增量变化，以便实现平滑的动画效果。
	 * @override 说明此函数是对其基类函数的重写。
	 */
	/*virtual void NativeUpdateAnimation(float DeltaSeconds) override;*/

	// 线程安全更新动画
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

private:
	// 角色的引用
	UPROPERTY(BlueprintReadOnly, Category="Character", meta=(AllowPrivateAccess=true))
	TObjectPtr<class ABlasterCharacter> BlasterCharacter = nullptr;

	// 角色的移动速度
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float Speed;

	// 角色是否在空中
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsInAir;

	// 角色是否在加速
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsAccelerating;

	// 角色是否装备武器
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bWeaponEquipped;

	// 角色装备的武器
	UPROPERTY()
	class AWeapon* EquippedWeapon = nullptr;

	// 角色是否瞄准
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bAiming;

	// YawOffset 表示在偏航方向上的偏移量。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float YawOffset;

	// Lean 表示倾斜角度。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float Lean;

	// 存储角色上一帧的旋转信息，用于计算旋转速度或进行插值等操作。
	FRotator CharacterRotationLastFrame;

	// 存储当前角色的旋转信息，表示角色当前的方向。
	FRotator CharacterRotation;

	// DeltaRotation 表示角色在当前帧与上一帧之间的旋转差值。
	FRotator DeltaRotation;

	// 存储角色的左右角偏移量，表示角色在左右的偏移量。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float AO_Yaw;

	// 存储角色的上下角偏移量，表示角色在上下的偏移量。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float AO_Pitch;

	// 存储角色的左手变换信息，表示角色的左手在当前帧的位置和旋转信息。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	FTransform LeftHandTransform;

	// 存储角色的转向状态，表示角色是否正在转弯。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	ETurningInPlace TurningInPlace;

	// 存储角色的右手变换信息，表示角色的右手在当前帧的位置和旋转信息。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	FRotator RightHandRotation;

	// 存储角色是否为本地玩家控制的信息，表示角色是否为本地玩家控制。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bLocallyControlled;

	// 存储角色是否需要旋转根骨骼的信息，表示角色是否需要旋转根骨骼。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bRotateRootBone;

	// 存储角色是否被淘汰的信息，表示角色是否被淘汰。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bEliminate;

	// 存储角色是否需要使用FABRIK算法的信息，表示角色是否需要使用FABRIK算法。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bUseFABRIK;

	// 存储角色是否需要使用瞄准偏移信息的信息，表示角色是否需要使用瞄准偏移信息。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bUseAimOffsets;

	// 存储角色是否需要使用右手偏移信息的信息，表示角色是否需要使用右手偏移信息。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bTransformRightHand;

	// 存储角色是否持有旗帜的信息，表示角色是否持有旗帜。
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bHoldingTheFlag;
};
