// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/LagCompensationComponent.h"

#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Weapon.h"


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
	SaveFramePackage();
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

// 保存当前帧的数据包
void ULagCompensationComponent::SaveFramePackage()
{
	// 确保拥有者是在服务器上
	if (Character == nullptr || !Character->HasAuthority()) return;

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
		Package.Character = Character;

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

// 添加帧数据包
void ULagCompensationComponent::AddFramePackage()
{
	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);
	FrameHistory.AddHead(ThisFrame);

	// ShowFramePackage(ThisFrame, FColor::Red);
}

// 服务器端回溯函数，用于处理命中补偿
/**             
 * 服务器端倒带（回退）
 * @param HitCharacter	命中的角色
 * @param TraceStart	命中的起始位置
 * @param HitLocation	命中的位置
 * @param HitTime		命中的时间
 */
FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	// 返回命中补偿结果
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

/**
 * 确认命中
 * @param Package			需要插值的帧数据包
 * @param HitCharacter		命中的角色
 * @param TraceStart		命中的起始位置
 * @param HitLocation		命中的位置
 * @return 
 */
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	// 如果击中的角色为空，则返回默认的服务器端回放结果
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	// 缓存当前帧的数据
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);

	// 移动碰撞盒到指定的位置
	MoveBoxes(HitCharacter, Package);

	// 禁用角色网格的碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// 首先启用头部碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 存储确认击中的结果
	FHitResult ConfirmHitResult;

	// 计算追踪结束的位置
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

	// 尝试进行线迹检测
	if (UWorld* World = GetWorld())
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility
		);
		// 击中了头部，没必要去检查其它Box了
		if (ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, true};
		}
		else // 没有击中头部
		{
			// 启用其他碰撞盒的碰撞检测
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
			// 再次进行线迹检测
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_Visibility);
			// 检查是否击中了其他盒子
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, false};
			}
		}
	}

	// 如果没有击中任何盒子，重置碰撞盒和角色网格的碰撞设置
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	// 返回表示没有击中的结果
	return FServerSideRewindResult{false, false};
}

/**
 * 执行散弹枪服务器端回放
 * 
 * @param HitCharacters 被击中的角色数组
 * @param TraceStart 射击的起始位置
 * @param HitLocation 被击中角色的位置数组
 * @param HitTime 击中时刻的时间戳
 * 
 * @return 返回射击结果
 * 
 * 此函数通过服务器端回放机制，确认散弹枪射击的命中情况
 * 它会遍历每个被击中的角色，并获取其在击中时刻的帧信息
 * 最后，根据这些帧信息和射击参数，确认射击是否命中
 */
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocation, float HitTime)
{
	// 存储需要检查的帧信息数组
	TArray<FFramePackage> FramesToCheck;
	// 遍历每个被击中的角色，获取其在击中时刻的帧信息
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	// 根据帧信息、射击起始位置和击中位置，确认射击结果
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocation);
}

// 确认散弹枪射击命中
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& Framepackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocation)
{
	return FShotgunServerSideRewindResult();
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	// 检查命中角色及其帧历史是否有效
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagComponent() == nullptr ||
		HitCharacter->GetLagComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagComponent()->FrameHistory.GetTail() == nullptr;

	// 如果有错误，返回默认值
	if (bReturn) FFramePackage();

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
		return FFramePackage();
	}

	// 如果命中时间与最旧历史时间相同，获取尾部历史帧，不需要插值
	if (OldestHistoyTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	// 如果命中时间晚于或等于最新的历史时间，获取头部历史帧，不需要插值
	if (NewestHistoyTime <= HitTime)
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
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return FrameToCheck;
}

/**
 * 伤害请求（只会在服务器调用）
 * @param HitCharacter	命中的角色
 * @param TraceStart	命中的起始位置
 * @param HitLocation	命中的位置
 * @param HitTime		命中的时间
 * @param DamageCauser	伤害的发起者
 */
void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
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

/** 缓存角色碰撞框位置信息
* 该函数用于将被击中角色的碰撞框位置信息存储到帧数据包中，以便于网络同步或回放
*  @param HitCharacter: 被击中的角色，其碰撞框信息将被缓存
*  @param OutFramePackage: 输出的帧数据包，将存储缓存的碰撞框信息
*/
void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	// 检查被击中的角色是否为空，如果为空则直接返回
	if (HitCharacter == nullptr) return;

	// 遍历被击中角色的所有碰撞框
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 检查当前碰撞框是否为空，如果为空则跳过
		if (HitBoxPair.Value != nullptr)
		{
			// 创建一个用于存储碰撞框位置信息的结构体
			FBoxInFormation BoxInfo;
			// 设置碰撞框的位置信息
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			// 设置碰撞框的旋转信息
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			// 设置碰撞框的大小信息
			BoxInfo.BoxExtent = HitBoxPair.Value->GetComponentScale();
			// 将碰撞框的信息添加到帧数据包中
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

// 移动角色身上的碰撞盒到指定位置和旋转
/**
 * @param HitCharacter 被击中的角色，如果为nullptr则直接返回
 * @param Package 包含了需要应用到碰撞盒上的位置、旋转和大小信息的帧包
 */
void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	// 检查被击中的角色是否为nullptr，如果是，则不执行任何操作
	if (HitCharacter == nullptr) return;

	// 遍历被击中角色的所有碰撞盒
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 检查当前碰撞盒是否为nullptr，如果是，则跳过
		if (HitBoxPair.Value != nullptr)
		{
			// 设置碰撞盒的世界位置
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			// 设置碰撞盒的世界旋转
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			// 设置碰撞盒的大小
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

/**
* 重置角色的碰撞盒位置和旋转
* @param HitCharacter 被击中的角色，如果为nullptr则直接返回
* @param Package 包含了需要应用到碰撞盒上的位置、旋转和大小信息的帧包
*/
void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	// 检查被击中的角色是否为nullptr，如果是，则不执行任何操作
	if (HitCharacter == nullptr) return;

	// 遍历被击中角色的所有碰撞盒
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 检查当前碰撞盒是否为nullptr，如果是，则跳过
		if (HitBoxPair.Value != nullptr)
		{
			// 设置碰撞盒的世界位置
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			// 设置碰撞盒的世界旋转
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			// 设置碰撞盒的大小
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			// 禁用碰撞
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		}
	}
}

// 启用角色的碰撞
void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnable);
	}
}
