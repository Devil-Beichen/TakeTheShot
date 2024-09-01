// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 *	玩家状态
 */
UCLASS()
class TAKETHESHOT_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// 重写分数发生变化
	virtual void OnRep_Score() override;

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

private:
	// 玩家角色
	UPROPERTY()
	class ABlasterCharacter* Character = nullptr;
	// 玩家控制器
	UPROPERTY()
	class ABlasterPlayerController* Controller = nullptr;
};
