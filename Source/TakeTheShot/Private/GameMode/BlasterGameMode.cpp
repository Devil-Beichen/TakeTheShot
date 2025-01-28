// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BlasterGameMode.h"

#include "Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlasterPlayerController.h"
#include "PlayerState/BlasterPlayerState.h"

namespace MatchState
{
	// 比赛为冷却状态
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	// 设置默认延迟启动（有预热时间）
	bDelayedStart = true;
	bLevelTimeInit = false;
}


void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bLevelTimeInit == false)
	{
		bLevelTimeInit = true;
		LevelStartingTime = GetWorld()->GetTimeSeconds();
		// GEngine->AddOnScreenDebugMessage(-1, 120, FColor::Orange, FString::Printf(TEXT("游戏模式...关卡启动的时间为%f"), LevelStartingTime));
	}
}

// 比赛状态设置
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
	// 如果比赛状态为进行中
	else if (MatchState == MatchState::InProgress)
	{
		// 计算倒计时时间，考虑到预热时间和比赛时间，以及当前世界时间和关卡开始时间
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		// 如果倒计时时间小于等于0，则表示比赛时间结束
		if (CountDownTime <= 0.f)
		{
			// 将比赛状态设置为冷却状态，准备进入下一阶段
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

// 玩家被淘汰
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	// 根据AttackerController是否存在，转换并获取相应的ABlasterPlayerState对象
	// 如果AttackerController为nullptr，则AttackerPlayerState为nullptr
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;

	// 类似地，根据VictimController是否存在，转换并获取相应的ABlasterPlayerState对象
	// 如果VictimController为nullptr，则VictimPlayerState为nullptr
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		// 获取当前拥有分数的玩家列表
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;

		// 遍历当前拥有分数的玩家列表
		for (auto& LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		// 为攻击者玩家状态增加分数
		AttackerPlayerState->AddToScore(1.f);
		// 更新游戏状态中的最高分数
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		// 如果当前拥有分数的玩家列表中包含当前攻击者玩家状态，则更新领先
		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			if (ABlasterCharacter* Loser = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn()))
			{
				Loser->MulticastGainedTheLead();
			}
		}

		// 如果当前拥有分数的玩家列表中不包含当前攻击者玩家状态，则更新失去领先
		for (auto& LeadPlayer : PlayersCurrentlyInTheLead)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(LeadPlayer))
			{
				if (ABlasterCharacter* Loser = Cast<ABlasterCharacter>(LeadPlayer->GetPawn()))
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	// 受害者玩家状态
	if (VictimPlayerState)
	{
		VictimPlayerState->AddTotDefeats();
	}

	// 如果被消除的角色对象存在
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim(false);
	}

	// 遍历所有玩家控制器
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It))
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState); // 广播被淘汰公告
		}
	}
}

// 请求重新生成
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

// 玩家离开游戏
void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	// 调用玩家淘汰，并且将bLeftGame设置成true
	if (ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>(); BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	if (ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn()))
	{
		CharacterLeaving->Elim(true);
	}
}

/**
* 计算实际受到的伤害
* @param Attacker		发起攻击的控制器
* @param Victim		被攻击的控制器
* @param BaseDamage	基础伤害
* @return 
*/
float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
