// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/BlasterPlayerState.h"

#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

// 添加分数
void ABlasterPlayerState::AddToScore(const float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	MySetScore();
}

// 设置分数
void ABlasterPlayerState::MySetScore()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(GetPawn()->Controller) : Controller;

	if (Character && Controller)
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
