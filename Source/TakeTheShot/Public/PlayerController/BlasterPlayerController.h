// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

// 高延迟多播委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**	爆破玩家的控制器
 * 
 */
UCLASS()
class TAKETHESHOT_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 设置HUD血量
	 * 
	 * @param Health		当前血量
	 * @param MaxHealth		最大血量
	 */
	void SetHUDHealth(const float Health, const float MaxHealth);

	/**
	 * 设置护盾
	 * @param Shield		当前护盾
	 * @param MaxShield		最大护盾
	 */
	void SetHUDShield(const float Shield, const float MaxShield);

	/**
	 * 设置HUD（Heads-Up Display）中的分数显示
	 * 
	 * @param Score 要设置的分数值，用于更新HUD中的显示内容
	 */
	void SetHUDScore(const float Score);

	/**
	 * 设置HUD上的击败数
	 * 
	 * @param Defeats 要设置的击败数
	 */
	void SetHUDDefeats(const int32 Defeats);

	/**	设置武器里的弹药（弹匣）
	 * 
	 * @param Ammo 弹药
	 */
	void SetHUDWeaponAmmo(const int32 Ammo);

	/**	设置携带的带药(备弹)
	 * 
	 * @param Ammo 弹药
	 */
	void SetHUDCarriedAmmo(const int32 Ammo);

	// 设置HUD上的匹配倒计时
	void SetHUDMatchCountdown(float CountdownTime);

	// 设置HUD上的公告倒计时
	void SetHUDAnnouncementCountdown(float CountdownTime);

	// 设置携带的手雷
	void SetHUDGrenades(const int32 Grenades);

	// 重写拥有pawn
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaSeconds) override;

	// 获取服务器的时间
	virtual float GetServerTime() const;

	// 在PlayerController的viewport/net连接与此播放器控制器关联后调用
	virtual void ReceivedPlayer() override; // 尽快与服务器时间同步

	// 匹配状态改变
	void OnMatchStateSet(FName State, bool bTeamsMate = false);

	// 到服务器的时间（单程）
	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	/**
	 * 广播玩家被淘汰
	 * @param Attacker		攻击者的状态
	 * @param Victim		被淘汰的玩家的状态
	 */
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

	// 隐藏队伍分数
	void HideTeamScores();
	// 初始化队伍分数
	void InitTeamScores();
	// 设置HUD的红队分数
	void SetHUDRedTeamScore(int32 RedScore);
	// 设置HUD的蓝队分数
	void SetHUDBlueTeamScore(int32 BlueScore);

protected:
	virtual void BeginPlay() override;

	// 设置HUD时间
	void SetHUDTime();

	// 初始化
	void PollInit();

	/**
	 * 服务器与客户端之间的时间同步
	 */

	/** 请求服务器的时间(服务端执行)
	 * @param TimeOfClientRequest 客户端请求的时间
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServetTime(const float TimeOfClientRequest);

	/**
	 * 向客户端报告当前服务器的时间
	 * 
	 * 该函数以可靠的方式（即数据包不会丢失）从客户端请求服务器时间这在同步时间或调整客户端操作时序时非常有用
	 * 
	 * @param TimeOfClientRequest 客户端发出请求的时间这个参数记录了客户端请求服务器时间的时间点
	 * @param TimeServerReceivedClientRequest 服务器接收到客户端请求的时间这个参数记录了服务器端接收到客户端时间请求的时间点
	 * 
	 * 注意：尽管这里提供了两个时间参数，但这个函数的实际作用可能是为了测量网络延迟或进行时间同步操作
	 */
	UFUNCTION(Client, Reliable)
	void ClientRequestServetTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);

	// 客户端与服务器之间的时间差
	float ClientServerDelta = 0.f;

	// 时间同步的频率
	UPROPERTY(EditDefaultsOnly, Category = Time)
	float TimeSyncFrequency = 5.f;

	// 时间同步的计时器
	float TimeSyncRunningTime = 0.f;

	// 检查时间同步
	void CheckTimeSync(float DeltaSeconds);

	// 服务器检查匹配状态
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// 匹配模式是否已经初始化
	uint8 bGameModeInit : 1;

	/**
	 * 客户端加入游戏
	 * 
	 * 此函数用于让客户端在游戏进行中加入，提供游戏状态和时间信息以同步客户端的状态。
	 * 
	 * @param StateOfMatch 游戏对局的状态名称，用于识别当前游戏处于哪个阶段。
	 * @param Warmup 游戏的热身时间，单位为秒，用于同步客户端的热身状态。
	 * @param Match 游戏的匹配时间，单位为秒，用于同步客户端的匹配状态。
	 * @param Cooldown 游戏的冷却时间，单位为秒，用于同步客户端的冷却状态。
	 * @param StartingTime 游戏的开始时间，单位为秒，用于同步客户端的游戏开始时间。
	 */
	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	// 显示延迟
	void ShowPingDisplay();

	// 检查延迟
	void CheckPing(float DeltaSeconds);

	// 显示高延迟
	void HighPingWarning();

	// 停止显示高延迟
	void StopHighPingWarning();

	/**
	 * 客户端显示淘汰公告
	 * @param Attacker		攻击者的状态 
	 * @param Victim		被淘汰的玩家的状态
	 */
	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	// 显示队伍分数
	UPROPERTY(Replicated)
	bool bShowTeamScores = false;

	// 获取队伍信息
	FString GetInfoText(const TArray<class ABlasterPlayerState*>& Players);

	// 获取队伍信息
	FString GetTeamsInfoText(class ABlasterGameState* BlasterGameState);

private:
	// HUD
	UPROPERTY()
	class ABlasterHUD* BlasterHUD = nullptr;

	// 游戏模式
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	// 玩家进入游戏时的时间
	float LevelStartingTime = 0.f;

	// 匹配倒计时
	float MatchTime = 0.f;
	// 预热时间
	float WarmupTime = 0.f;

	// 结束的冷却时间
	float CooldownTime = 0.f;

	// 倒计时整数
	uint32 CountdownInt = 0;

	// 匹配状态
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	// 匹配状态改变(只会在客户端回调)
	UFUNCTION()
	void OnRep_MatchState();

	// 设置匹配状态
	void SetMatchState();

	// 匹配开始函数
	void HandleMatchHasStarted();

	// 处理冷却时间函数
	void HandleCooldown();

	// 玩家UI
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	// 初始化血量
	bool bInitializeHealth = false;
	// HUD血量
	float HUDHealth;
	float HUDMaxHealth;

	// 初始化护盾
	bool bInitializeShield = false;
	// HUD护盾
	float HUDShield;
	float HUDMaxShield;

	// 初始化分数
	bool bInitializeScore = false;
	// HUD分数
	float HUDScore;

	// 初始化死亡次数
	bool bInitializeDefeat = false;
	int32 HUDDefeats;

	// 初始化手雷数量
	bool bInitializeGrenade = false;
	// HUD携带的手雷数量
	int32 HUDGrenades;

	// 初始化备用弹药
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	// 初始化武器弹药
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	// 高延迟提示的时间
	float HighPingRunningTime = 0.f;

	// 高延迟提示的持续时间
	UPROPERTY(EditDefaultsOnly, Category = Time)
	float HighPingDuration = 5.f;

	// 高延迟动画的持续时间
	float PingAnimationRunningTime = 0.f;

	// 检查延迟的频率
	UPROPERTY(EditDefaultsOnly, Category = Time)
	float CheckPingFrequency = 20.f;

	// 报告延迟状态
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHightPing);

	// 高延迟的阈值
	UPROPERTY(EditDefaultsOnly, Category = Time)
	float HighPingThreshold = 100.f;
};
