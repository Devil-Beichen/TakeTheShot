// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

ALobbyGameMode::ALobbyGameMode()
{
	// 游戏是否使用在后台加载的SeamlessTravel()执行地图旅行，并且不会断开客户端连接
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 获取当前游戏状态中玩家的数量
	// 通过GameState的Get方法获取全局游戏状态实例，然后调用其PlayerArray属性的Num方法来计算玩家数量
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayer == 2)
	{
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}
}
