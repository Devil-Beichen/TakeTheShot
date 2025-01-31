// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/Team.h"
#include "GameFramework/PlayerStart.h"
#include "TeamPlayerStart.generated.h"

/**
 * 团队模式玩家开始位置
 */
UCLASS()
class TAKETHESHOT_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	// 团队
	UPROPERTY(EditDefaultsOnly)
	ETeam Team = ETeam::ET_NoTeam;
};
