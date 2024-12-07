// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}

// 恢复生命
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

// 治疗增加
void UBuffComponent::HealRampUp(float DetaTime)
{
	if (!bHealing || Character == nullptr || Character->IsEliminate()) return;
	// 没帧治愈的血量
	const float HealThisFrame = HealingRate * DetaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	AmountToHeal -= HealThisFrame;
	Character->UpdateHUDHealth();
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

// 初始化基础速度
void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

// 获得速度 buff
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTime,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);
	SetSpeed(BuffBaseSpeed, BuffCrouchSpeed); // 在服务器上设置速度(是加速)
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed); // 多播设置速度(是加速)
}

// 重置速度
void UBuffComponent::ResetSpeeds()
{
	SetSpeed(InitialBaseSpeed, InitialCrouchSpeed, false); // 不是加速
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed, false);
}

// 多播设置速度
void UBuffComponent::MulticastSpeedBuff_Implementation(float BuffBaseSpeed, float BuffCrouchSpeed, bool bAccelebuff)
{
	SetSpeed(BuffBaseSpeed, BuffCrouchSpeed, bAccelebuff);
}

// 设置速度
void UBuffComponent::SetSpeed(float BaseSpeed, float CrouchSpeed, bool bAccelebuff)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr || Character->IsEliminate()) return;
	bAccelerationbuff = bAccelebuff;
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	if (bAccelebuff)
	{
		// 如果在加速的情况下就取消慢速
		Character->bIsSlowWalk = false;
	}
}

// 初始化跳跃速度
void UBuffComponent::SetInitialJumpVelocity(float BaseJumpVelocity)
{
	InitialJumpVelocity = BaseJumpVelocity;
}

// 跳跃 buff
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	SetJumpVelocity(BuffJumpVelocity);
	MulticastJumpVelocity(BuffJumpVelocity);
}

// 重置跳跃速度
void UBuffComponent::ResetJump()
{
	SetJumpVelocity(InitialJumpVelocity);
	MulticastJumpVelocity(InitialJumpVelocity);
}

// 设置跳跃速度
void UBuffComponent::SetJumpVelocity(float BuffJumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
}

// 多播设置跳跃速度
void UBuffComponent::MulticastJumpVelocity_Implementation(float BuffJumpVelocity)
{
	SetJumpVelocity(BuffJumpVelocity);
}
