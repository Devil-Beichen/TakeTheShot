// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterPlayerState;

namespace MatchState
{
	// 比赛时间已到，显示获胜者并且开始冷却定时器
	extern TAKETHESHOT_API const FName Cooldown;
}


/**
 *  爆破游戏模式
 */
UCLASS()
class TAKETHESHOT_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();

	virtual void Tick(float DeltaSeconds) override;

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

	// 玩家离开游戏
	void PlayerLeftGame(ABlasterPlayerState* PlayerLeaving);

	/**
	 * 计算实际受到的伤害
	 * @param Attacker		发起攻击的控制器
	 * @param Victim		被攻击的控制器
	 * @param BaseDamage	基础伤害
	 * @return 
	 */
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	// 热身时间
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	// 游戏时间
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	// 关卡开始的时间
	float LevelStartingTime = 0.f;

	// 关卡时间初始化
	uint8 bLevelTimeInit : 1;

	// 冷却时间
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	// 是否为团队模式
	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;

	// 匹配状态设置
	virtual void OnMatchStateSet() override;

private:
	// 倒计时时间
	float CountDownTime = 0.f;

public:
	// 获取倒计时时间
	FORCEINLINE float GetCountdownTime() const { return CountDownTime; }
};
