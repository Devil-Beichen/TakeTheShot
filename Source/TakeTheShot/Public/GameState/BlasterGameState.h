// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"


class ABlasterPlayerState;

/**
 * 游戏状态
 */
UCLASS()
class TAKETHESHOT_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	/**
	 * 更新最高分数
	 * 
	 * 此函数用于更新游戏中最高分数的记录它会将传入的玩家得分
	 * 与当前的最高分数进行比较，如果更高，则更新最高分数
	 * 
	 * @param ScoringPlayer 指向玩家状态的指针，该玩家的得分将被用来尝试更新最高分数
	 */
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	// 玩家状态
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	/**
	 * 团队
	 */

	// 红队得分
	void RedTeamScores();
	// 蓝队得分
	void BlueTeamScores();

	// 红队状态
	TArray<ABlasterPlayerState*> RedTeam;
	// 蓝队状态
	TArray<ABlasterPlayerState*> BlueTeam;

	// 红队得分
	UPROPERTY(ReplicatedUsing = "OnRep_RedTeamScore")
	float RedTeamScore = 0.f;
	// 蓝队得分
	UPROPERTY(ReplicatedUsing = "OnRep_BlueTeamScore")
	float BlueTeamScore = 0.f;

	// 红队得分更新
	UFUNCTION()
	void OnRep_RedTeamScore();

	// 蓝队得分更新
	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	// 最高分
	float TopScore = 0.f;
};
