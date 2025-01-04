// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/LagCompensationComponent.h"

#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.Num() <= 1)
	{
		AddFramePackage();
	}
	else
	{
		// 获取历史帧数据包持续的时间长度（用最新的时间减去最旧的时间）
		float HistoryLength = GetFrameTime();
		while (HistoryLength > MaxRecordTime)
		{
			// 移除最旧的帧数据包
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = GetFrameTime();
		}
		AddFramePackage();
	}
}

// 添加帧数据包
void ULagCompensationComponent::AddFramePackage()
{
	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);
	FrameHistory.AddHead(ThisFrame);

	ShowFramePackage(ThisFrame, FColor::Red);
}

// 获取帧数据包持续的时间长度
float ULagCompensationComponent::GetFrameTime() const
{
	return FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
}

// 显示帧数据包
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, FColor Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f

		);
	}
}

// 存储当前帧的数据
// 保存帧数据包
// 该函数用于捕获当前游戏世界的时间以及角色碰撞盒的信息，并将这些信息存储在FFramePackage对象中
// 这对于 lag compensation（延迟补偿）机制非常关键，因为它允许游戏在处理网络延迟的情况下更准确地判断命中检测
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	// 获取或更新角色引用
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;

	// 如果角色有效，则继续保存帧数据
	if (Character)
	{
		// 记录当前世界时间
		Package.Time = GetWorld()->GetTimeSeconds();

		// 遍历角色的每个碰撞盒，保存其位置、旋转和扩展信息
		for (auto& BoxPaor : Character->HitCollisionBoxes)
		{
			FBoxInFormation BoxInFormation;
			BoxInFormation.Location = BoxPaor.Value->GetComponentLocation();
			BoxInFormation.Rotation = BoxPaor.Value->GetComponentRotation();
			BoxInFormation.BoxExtent = BoxPaor.Value->GetScaledBoxExtent();

			// 将碰撞盒信息添加到帧数据包中
			Package.HitBoxInfo.Add(BoxPaor.Key, BoxInFormation);
		}
	}
}
