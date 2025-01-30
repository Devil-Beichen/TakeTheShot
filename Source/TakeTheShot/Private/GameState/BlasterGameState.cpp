// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/BlasterGameState.h"

#include "Net/UnrealNetwork.h"
#include "PlayerState/BlasterPlayerState.h"


void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

// 更新最高分数
void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

// 红队得分
void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
}

// 蓝队得分
void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
}

// 红队得分更新
void ABlasterGameState::OnRep_RedTeamScore()
{
}

// 蓝队得分更新
void ABlasterGameState::OnRep_BlueTeamScore()
{
}
