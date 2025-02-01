// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CaptureTheFlagGameMode.h"

#include "CaotureTheFlag/FlagZone.h"
#include "GameState/BlasterGameState.h"
#include "Weapon/Flag.h"

// 重写玩家死亡事件
void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

/**
 * 团队获取旗子
 * @param Flag 旗子
 * @param Zone 区域
 */
void ACaptureTheFlagGameMode::FlagCaptured(class AFlag* Flag, class AFlagZone* Zone)
{
	// 是否有效获取旗子
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState); bValidCapture) // 有效获取
	{
		// 根据旗子的队伍设置对应的团队得分
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
		else if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
	}
}
