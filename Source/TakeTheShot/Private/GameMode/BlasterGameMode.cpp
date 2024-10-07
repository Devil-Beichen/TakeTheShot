// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BlasterGameMode.h"

#include "Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlasterPlayerController.h"
#include "PlayerState/BlasterPlayerState.h"

ABlasterGameMode::ABlasterGameMode()
{
	// 设置默认延迟启动（有预热时间）
	bDelayedStart = true;
}


void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It))
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 当比赛状态为等待开始时
	if (MatchState == MatchState::WaitingToStart)
	{
		// 计算倒计时时间，直到比赛开始
		 CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		// 如果倒计时结束
		if (CountDownTime <= 0.f)
		{
			// 开始比赛
			StartMatch();
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// 根据AttackerController是否存在，转换并获取相应的ABlasterPlayerState对象
	// 如果AttackerController为nullptr，则AttackerPlayerState为nullptr
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;

	// 类似地，根据VictimController是否存在，转换并获取相应的ABlasterPlayerState对象
	// 如果VictimController为nullptr，则VictimPlayerState为nullptr
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	// 受害者玩家状态
	if (VictimPlayerState)
	{
		VictimPlayerState->AddTotDefeats();
	}

	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	// 如果被消除的角色对象存在
	if (EliminatedCharacter)
	{
		// 重置角色对象，准备重新开始
		EliminatedCharacter->Reset();
		// 销毁角色对象，释放资源
		EliminatedCharacter->Destroy();
	}

	// 如果被消除的控制器对象存在
	if (EliminatedController)
	{
		// 找到所有玩家起点对象
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		// 随机选择一个玩家起点
		const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		// 让被消除的玩家在新的起点重新开始游戏
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}
