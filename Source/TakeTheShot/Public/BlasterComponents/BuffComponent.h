// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

/**
 * buff组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TAKETHESHOT_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();

	/* 定义一个友元类ABlasterCharacter
	友元类可以访问此函数所属类的私有和保护成员
	这里声明友元类是为了在ABlasterCharacter类中访问当前类的某些成员或函数
	*/
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 恢复生命
	 * @param HealAmount	恢复的总生命值 
	 * @param HealingTime	恢复持续的时间
	 */
	void Heal(float HealAmount, float HealingTime);

	/**
	 *  获得速度buff
	 * @param BuffBaseSpeed		基础速度
	 * @param BuffCrouchSpeed	蹲下的速度
	 * @param BuffTime			buff持续时间
	 */
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);

	// 设置初始速度
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	// 设置初始跳跃速度
	void SetInitialJumpVelocity(float BaseJumpVelocity);

	/**
	*	跳跃BUFF
	* @param BuffJumpVelocity		跳跃速度
	* @param BuffTime				buff持续时间
	*/
	void BuffJump(float BuffJumpVelocity, float BuffTime);

protected:
	virtual void BeginPlay() override;

	// 治疗增加
	void HealRampUp(float DetaTime);

private:
	// 定义一个指向Blaster角色的指针，用于在装备系统中引用角色实例
	UPROPERTY()
	ABlasterCharacter* Character = nullptr;

	/**
	 * 生命相关的BUFF
	 */

	// 用于记录是否正在恢复生命
	bool bHealing = false;
	// 目前恢复的速率
	float HealingRate = 0.f;
	// 还需要恢复的
	float AmountToHeal = 0.f;

	/**
	 * 速度相关的BUFF
	 */

	// 速度Buff的定时器
	FTimerHandle SpeedBuffTime;

	// 重置速度
	void ResetSpeeds();

	// 加速buff
	bool bAccelerationbuff = false;

	// 初始基础速度
	float InitialBaseSpeed = 0.f;

	// 初始蹲下速度
	float InitialCrouchSpeed = 0.f;

	// 多播设置速度
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, bool bAccelebuff = true);

	/**
	 * 
	 * @param BaseSpeed			基础速度
	 * @param CrouchSpeed 		蹲下的速度
	 * @param bAccelebuff		是否是加速buff
	 */
	void SetSpeed(float BaseSpeed, float CrouchSpeed, bool bAccelebuff = true);

	/**
	 *  跳跃相关的BUFF
	 */

	// 跳跃buff的定时器
	FTimerHandle JumpBuffTimer;

	// 重置跳跃
	void ResetJump();

	// 初始跳跃速度
	float InitialJumpVelocity = 0.f;

	/**
	 * 设置跳跃速度
	 * @param BuffJumpVelocity 	跳跃速度
	 */
	void SetJumpVelocity(float BuffJumpVelocity);

	/**
	 * 
	 * @param BuffJumpVelocity 	跳跃速度
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpVelocity(float BuffJumpVelocity);

public:
};
