// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterTypes/Team.h"
#include "BlasterPlayerState.generated.h"

/**
 *	玩家状态
 */
UCLASS()
class TAKETHESHOT_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/**
	 *  复制 通知
	 */

	// 重写分数发生变化
	virtual void OnRep_Score() override;
	// 死亡回调
	UFUNCTION()
	virtual void OnRep_Defeats();

	/**
	 * 向得分添加指定数量的分数
	 * 
	 * 此函数的目的是为了提供一种方式，使得在游戏中可以动态地为玩家得分增加分数，从而增强游戏的互动性和趣味性
	 * 
	 * @param ScoreAmount 要添加到当前得分的分数值，应该为正数以增加分数，负数以减少分数
	 */
	void AddToScore(const float ScoreAmount);

	// 我设置分数
	void MySetScore();

	// 增加死亡次数
	void AddTotDefeats(const int32 DefeatsAmount = 1);

	// 设置死亡次数
	void SetDefeats();

private:
	// 玩家角色
	UPROPERTY()
	class ABlasterCharacter* Character = nullptr;
	// 玩家控制器
	UPROPERTY()
	class ABlasterPlayerController* Controller = nullptr;

	// 死亡
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats = 0;

	// 队伍
	UPROPERTY(ReplicatedUsing = "OnRep_Team")
	ETeam Team = ETeam::ET_NoTeam;

	// 队伍发生改变
	UFUNCTION()
	void OnRep_Team();

public:
	// 获取队伍
	FORCEINLINE ETeam GetTeam() const { return Team; }
	// 设置队伍
	void SetTeam(ETeam TeamToSet);
};
