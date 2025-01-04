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

// 服务器端回溯函数，用于处理命中补偿
/**             
 * 服务器端倒带（回退）
 * @param HitCharacter	命中的角色
 * @param TraceStart	命中的起始位置
 * @param HitLocation	命中的位置
 * @param HitTime		命中的时间
 */
void ULagCompensationComponent::ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	// 检查命中角色及其帧历史是否有效
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagComponent() == nullptr ||
		HitCharacter->GetLagComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagComponent()->FrameHistory.GetTail() == nullptr;

	// 初始化要检查的帧
	FFramePackage FrameToCheck;

	// 初始化是否应该插值的标志
	bool bShouldInterpolate = true;

	// 获取命中角色的帧历史
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagComponent()->FrameHistory;
	// 获取最早和最新的帧历史时间
	const float OldestHistoyTime = History.GetTail()->GetValue().Time;
	const float NewestHistoyTime = History.GetHead()->GetValue().Time;

	// 如果命中时间比最早的历史时间还早，直接返回
	if (OldestHistoyTime > HitTime)
	{
		return;
	}

	// 如果命中时间等于最早或最新历史时间，设置要检查的帧并禁用插值
	if (OldestHistoyTime == HitTime || NewestHistoyTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	// 初始化用于插值的较新和较旧帧指针
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	// 遍历历史帧数据包，找到最接近命中时间的帧数据包
	while (Older->GetValue().Time > HitTime)
	{
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}

	// 如果找到与命中时间完全匹配的帧，设置要检查的帧并禁用插值
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	// 如果需要插值，执行插值操作
	if (bShouldInterpolate)
	{
		// 比较老的时间和比较新的时间之间进行插值
	}

	// 如果之前标记为返回，则返回
	if (bReturn) return;
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

/**
 * 在两帧之间进行插值计算
 * 
 * 此函数用于根据给定的两个帧（OlderFrame 和 YoungerFram）以及一个指定的时间点（HitTime），
 * 计算出在这个时间点上，各个命中盒（HitBox）的位置和旋转状态此方法主要用于补偿网络延迟下的画面显示问题，
 * 通过插值处理来平滑动画效果
 * 
 * @param OlderFrame 较旧的帧数据，用于插值计算的起始点
 * @param YoungerFram 较新的帧数据，用于插值计算的终点
 * @param HitTime 插值计算的目标时间点，决定了插值的结果
 * @return 返回插值计算得到的帧数据，包括各个命中盒在目标时间点的位置和旋转状态
 */
FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFram, float HitTime)
{
    // 计算两帧之间的时间距离
    const float Distance = YoungerFram.Time - OlderFrame.Time;
    // 计算插值比例，确保其在0到1之间，避免超出有效范围
    const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

    // 初始化插值计算得到的帧数据结构
    FFramePackage InterpFramePackage;
    InterpFramePackage.Time = HitTime;
    // 遍历较新的帧中的每个命中盒信息，进行插值计算
    for (auto& YoungerPair : YoungerFram.HitBoxInfo)
    {
        // 获取命中盒的名称，用于索引和匹配
        const FName& BoxInfoName = YoungerPair.Key;
        // 从较旧和较新的帧数据中获取对应的命中盒信息
        const FBoxInFormation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
        const FBoxInFormation& YoungerBox = YoungerFram.HitBoxInfo[BoxInfoName];

        // 初始化插值计算得到的命中盒信息
        FBoxInFormation InterpBoxInfo;
        // 使用线性插值计算命中盒的位置
        InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
        // 使用线性插值计算命中盒的旋转
        InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
        // 直接使用较新的帧中的命中盒尺寸信息，因为尺寸不参与插值计算
        InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;
        // 将插值计算得到的命中盒信息添加到结果帧数据中
        InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
    }
    // 返回插值计算得到的帧数据
    return InterpFramePackage;
}

