// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"

#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

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
}
