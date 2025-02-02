// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"

#include "ShotUISubsystem.h"
#include "GameFramework/GameStateBase.h"

ALobbyGameMode::ALobbyGameMode()
{
	// 游戏是否使用在后台加载的SeamlessTravel()执行地图旅行，并且不会断开客户端连接
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	EMatchType MatchType{};
	UWorld* World = GetWorld();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UShotUISubsystem* ShotUISubsystem = GameInstance->GetSubsystem<UShotUISubsystem>();
		MatchType = ShotUISubsystem->DesiredMatchType;
	}

	// 获取当前游戏状态中玩家的数量
	// 通过GameState的Get方法获取全局游戏状态实例，然后调用其PlayerArray属性的Num方法来计算玩家数量
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayer == 2)
	{
		switch (MatchType)
		{
		case EMatchType::EM_FreeForAll:
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
			break;
		case EMatchType::EM_Teams:
			World->ServerTravel(FString("/Game/Maps/TeamMap?listen"));
			break;
		case EMatchType::EM_CaptureTheFlag:
			World->ServerTravel(FString("/Game/Maps/FlagMap?listen"));
			break;
		case EMatchType::EM_MAX:
			break;
		}
	}
}
