// Fill out your copyright notice in the Description page of Project Settings.


#include "CaotureTheFlag/FlagZone.h"

#include "Components/SphereComponent.h"
#include "GameMode/CaptureTheFlagGameMode.h"
#include "Weapon/Flag.h"


AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
	ZoneSphere->SetCollisionObjectType(ECC_WorldDynamic);
	ZoneSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	GameMode = GameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>() : GameMode;

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

/**
* 当角色的碰撞球体与另一个演员的碰撞体发生重叠时，此函数将被调用。
 * 
* @param OverlappedComponent 角色的碰撞球体组件，表示触发重叠的组件。
* @param OtherActor 发生重叠的另一个演员。
* @param OtherComp 另一个演员的碰撞组件。
* @param OtherBodyIndex 另一个碰撞体的索引。
* @param bFromSweep 如果重叠是由于扫动检测引起的，则此参数为true；否则为false。
* @param SweepResult 扫动检测的结果，包含重叠的详细信息。
*/
void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 尝试将其他演员转换为旗子，并检查是否成功且不属于同一团队
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		// 获取游戏模式实例，如果尚未获取
		GameMode = GameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>() : GameMode;
		// 如果游戏模式实例已获取，则通知游戏模式旗子被捕获
		if (GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag(); // 重置旗子
	}
}
