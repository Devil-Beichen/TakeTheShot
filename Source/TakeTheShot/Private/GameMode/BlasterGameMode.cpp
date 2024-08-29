// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BlasterGameMode.h"

#include "Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	// 如果被消除的角色对象存在
	if (EliminatedCharacter)
	{
	    // 重置角色对象，准备重新开始
	    EliminatedCharacter->Reset();
	    // 销毁角色对象，释放资源
	    EliminatedCharacter->Destroy();
	}
	
	// 如果被消除的控制器对象存在
	if (EliminatedController)
	{
	    // 找到所有玩家起点对象
	    TArray<AActor*> PlayerStarts;
	    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	
	    // 随机选择一个玩家起点
	    const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
	    // 让被消除的玩家在新的起点重新开始游戏
	    RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}
