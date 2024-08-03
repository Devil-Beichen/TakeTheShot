// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

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

	// 角色是否瞄准
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bAiming;
};
