// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 团队游戏模式
 */
UCLASS()
class TAKETHESHOT_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	// 玩家加入时
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 玩家退出时
	virtual void Logout(AController* Exiting) override;

	// 从写计算伤害
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;

protected:
	// 匹配开始时
	virtual void HandleMatchHasStarted() override;

	// 团队分组
	void TeamGroup();
};
