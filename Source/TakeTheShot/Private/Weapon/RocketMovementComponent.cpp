// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapon/RocketMovementComponent.h"


UProjectileMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	// 返回指示处理碰撞事件的结果，决定是否继续执行下一个子步骤
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// 火箭弹不应当停下来，只有当她们的碰撞盒体检测到撞机才会爆炸
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}
