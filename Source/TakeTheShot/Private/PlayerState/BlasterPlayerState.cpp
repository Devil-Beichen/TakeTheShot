// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/BlasterPlayerState.h"

#include "Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlasterPlayerController.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

// 添加分数
void ABlasterPlayerState::AddToScore(const float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	MySetScore();
}

// 设置分数
void ABlasterPlayerState::MySetScore()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(GetPawn()->Controller) : Controller;

	if (Controller)
	{
		Controller->SetHUDScore(GetScore());
	}
}

// 分数改变的时候客户端回调
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	MySetScore();
}

// 添加死亡
void ABlasterPlayerState::AddTotDefeats(const int32 DefeatsAmount)
{
	// 累加失败次数
	Defeats += DefeatsAmount;

	// 更新HUD上的失败次数
	SetDefeats();
}

// 设置死亡
void ABlasterPlayerState::SetDefeats()
{
	// 检查并初始化控制器指针
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(GetPawn()->Controller) : Controller;

	// 如果控制器有效，则更新HUD上的失败次数
	if (Controller)
	{
		Controller->SetHUDDefeats(Defeats);
	}
}

// 死亡发生改变的时候客户端回调
void ABlasterPlayerState::OnRep_Defeats()
{
	// 当失败次数复制时，更新HUD显示
	SetDefeats();
} 
