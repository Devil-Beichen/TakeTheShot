// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

/**
 *  爆破游戏模式
 */
UCLASS()
class TAKETHESHOT_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	/**
	 * 当玩家被消灭时调用此函数，用于处理消灭相关的逻辑。
	 * 
	 * @param EliminatedCharacter 被消灭的角色对象，类型为ABlasterCharacter*。
	 * @param VictimController 被消灭玩家的控制器对象，类型为ABlasterPlayerController*。
	 * @param AttackerController 攻击者的控制器对象，类型为ABlasterPlayerController*。
	 */
	virtual void PlayerEliminated(
		ABlasterCharacter* EliminatedCharacter,
		ABlasterPlayerController* VictimController,
		ABlasterPlayerController* AttackerController
	);

	/** 请求复活角色
	 * 当一个角色被消除时，调用此函数来请求复活。
	 * 
	 * @param EliminatedCharacter 被消除的角色对象指针。
	 * @param EliminatedController 被消除角色的控制器对象指针。
	 */
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
};