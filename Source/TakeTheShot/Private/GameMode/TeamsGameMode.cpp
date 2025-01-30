// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"

#include "GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlasterPlayerController.h"
#include "PlayerState/BlasterPlayerState.h"

ATeamsGameMode::ATeamsGameMode()
{
	// 默认启用团队比赛
	bTeamsMatch = true;
}

// 玩家登录时
void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 获取游戏状态
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		// 将玩家状态转换为Blaster玩家状态，并检查玩家是否尚未分配团队
		if (ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
			BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			// 如果蓝队的人数大于等于红队的人数，则将玩家分配到红队，否则分配到蓝队
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BPState->SetTeam(ETeam::ET_RedTeam);
				BGameState->RedTeam.AddUnique(BPState);
			}
			else
			{
				BPState->SetTeam(ETeam::ET_BlueTeam);
				BGameState->BlueTeam.AddUnique(BPState);
			}
		}
	}
}

// 玩家退出时
void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	// 获取游戏状态
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		// 获取退出玩家的状态
		if (ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>())
		{
			// 检查并移除玩家状态
			if (BGameState->RedTeam.Find(BPState))
			{
				BGameState->RedTeam.Remove(BPState);
			}
			else if (BGameState->BlueTeam.Find(BPState))
			{
				BGameState->BlueTeam.Remove(BPState);
			}
		}
	}
}

// 计算伤害
float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	// 获取攻击者和 Victim 的玩家状态
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();

	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;

	// 如果攻击者和 Victim 的团队相同，则返回 0，表示伤害为 0
	if (AttackerPState->GetTeam() == VictimPState->GetTeam()) return 0.f;
	return BaseDamage;
}

// 玩家被淘汰时
void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
	// 获取游戏状态
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		else if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}

// 比赛开始时
void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	TeamGroup();
}

/**
 * 该函数用于在游戏模式中对玩家进行团队分组。
 * 它根据当前游戏状态中的玩家数量，将未分配团队的玩家分配到人数较少的团队中。
 */
void ATeamsGameMode::TeamGroup()
{
	// 获取游戏状态
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		// 遍历游戏状态中的所有玩家
		for (auto& PState : BGameState->PlayerArray)
		{
			// 将玩家状态转换为Blaster玩家状态，并检查玩家是否尚未分配团队
			if (ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
				BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				// 如果蓝队的人数大于等于红队的人数，则将玩家分配到红队，否则分配到蓝队
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BPState->SetTeam(ETeam::ET_RedTeam);
					BGameState->RedTeam.AddUnique(BPState);
				}
				else
				{
					BPState->SetTeam(ETeam::ET_BlueTeam);
					BGameState->BlueTeam.AddUnique(BPState);
				}
			}
		}
	}
}
