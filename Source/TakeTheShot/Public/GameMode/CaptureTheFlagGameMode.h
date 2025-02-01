// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 夺旗游戏模式
 */
UCLASS()
class TAKETHESHOT_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:
	// 重写玩家死亡事件
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;

	/**
	 * 团队获取旗子
	 * @param Flag	旗子 
	 * @param Zone	区域
	 */
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};
