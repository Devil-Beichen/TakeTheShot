// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"

#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameMode/BlasterGameMode.h"
#include "HUD/Announcement.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "BlasterComponents/CombatComponent.h"
#include "Components/Image.h"
#include "GameState/BlasterGameState.h"
#include "PlayerState/BlasterPlayerState.h"
#include "BlasterTypes/Announcement.h"

/**
 * 广播玩家被淘汰
 * @param Attacker		攻击者的状态
 * @param Victim		被淘汰的玩家的状态
 */
void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

/**
 * 客户端显示淘汰公告
 * @param Attacker		攻击者的状态 
 * @param Victim		被淘汰的玩家的状态
 */
void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			if (Attacker == Self && Victim != Self) // 如果攻击者是自己
			{
				BlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self) // 受到攻击的是自己
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self) // 自己伤害自己
			{
				BlasterHUD->AddElimAnnouncement("You", "Your Self");
				return;
			}
			if (Attacker == Victim && Attacker != Self) // 敌人自己伤害自己
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Themselves");
				return;
			}
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	if (!HasAuthority())
	{
		ServerCheckMatchState();
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState)
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores)
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && bGameModeInit == false && BlasterGameMode)
	{
		if (BlasterGameMode->bLevelTimeInit)
		{
			bGameModeInit = true;
			ServerCheckMatchState();
		}
	}
	CheckTimeSync(DeltaSeconds);
	PollInit();
	SetHUDTime();

	ShowPingDisplay();

	CheckPing(DeltaSeconds);
}

// 玩家被控制的回调函数
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		BlasterCharacter->Initialize();
	}
}

// 设置HUD倒计时
void ABlasterPlayerController::SetHUDTime()
{
	// 初始化剩余时间
	float TimeLeft = 0.f;

	// 根据比赛状态计算剩余时间
	if (MatchState == MatchState::WaitingToStart)
	{
		// 如果比赛处于等待开始状态，计算暖-up时间减去服务器时间再加上关卡开始时间
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		// 如果比赛处于进行中状态，计算比赛时间加上暖-up时间减去服务器时间再加上关卡开始时间
		TimeLeft = MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
	}

	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
	}

	/*GEngine->AddOnScreenDebugMessage(2, 0.1, HasAuthority() ? FColor::Blue : FColor::Green,
	                                 FString::Printf(TEXT("预热时间 %f \n 服务器时间 %f \n 关卡开始时间%f \n 倒计时时间%f"),
	                                                 WarmupTime, GetServerTime(), LevelStartingTime, TimeLeft));*/

	// 计算剩余秒数
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	// 如果当前对象拥有权威性（例如，在服务器上）
	if (HasAuthority())
	{
		// 初始化或更新BlasterGameMode对象，确保它不为nullptr
		// 如果BlasterGameMode为nullptr，则通过UGameplayStatics::GetGameMode(this)获取当前游戏模式，并尝试将其转换为ABlasterGameMode类型
		// 否则，使用已有的BlasterGameMode对象
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;

		// 如果BlasterGameMode对象有效
		if (BlasterGameMode)
		{
			// 计算剩余秒数，四舍五入到最近的整数
			// 这里的计算是将当前关卡开始时间与游戏模式的倒计时时间相加后的结果进行上限取整
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	// 如果剩余秒数有变化
	if (CountdownInt != SecondsLeft)
	{
		// 如果比赛处于等待开始或者结束状态
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			// 设置HUD公告倒计时
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		// 如果比赛处于进行中状态
		if (MatchState == MatchState::InProgress)
		{
			// 设置HUD比赛倒计时
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	// 更新倒计时整数秒数
	CountdownInt = SecondsLeft;
}

// 初始化
void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				// 初始化血量
				if (bInitializeHealth) { SetHUDHealth(HUDHealth, HUDMaxHealth); }
				// 初始化护盾
				if (bInitializeShield) { SetHUDShield(HUDShield, HUDMaxShield); }
				// 初始化分数
				if (bInitializeScore) { SetHUDScore(HUDScore); }
				// 初始化死亡
				if (bInitializeDefeat) { SetHUDDefeats(HUDDefeats); }
				// 初始化武器弹药
				if (bInitializeWeaponAmmo) { SetHUDWeaponAmmo(HUDWeaponAmmo); }
				// 初始化携带弹药
				if (bInitializeCarriedAmmo) { SetHUDCarriedAmmo(HUDCarriedAmmo); }
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if (bInitializeGrenade) { SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades()); }
				}
			}
		}
	}
}

// 检查时间同步
void ABlasterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if (TimeSyncRunningTime >= TimeSyncFrequency && IsLocalController())
	{
		ServerRequestServetTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

// 显示Ping
void ABlasterPlayerController::ShowPingDisplay()
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->PingText &&
		BlasterHUD->CharacterOverlay->FPSText;
	if (bHUDValid)
	{
		if (PlayerState)
		{
			// 计算健康百分比并更新健康条
			const float Ping = PlayerState->GetPingInMilliseconds();
			// 计算颜色
			float Alpha = FMath::GetMappedRangeValueClamped(FVector2d(30.f, 60.f), FVector2d(0.f, 1.f), Ping);
			// 颜色
			FLinearColor Color = FLinearColor::Green + Alpha * (FLinearColor::Red - FLinearColor::Green);
			// 设置颜色
			BlasterHUD->CharacterOverlay->PingText->SetColorAndOpacity(Color);

			// 显示Ping
			const FString PingText = FString::Printf(TEXT("Ping : %d"), FMath::RoundToInt32(Ping));
			BlasterHUD->CharacterOverlay->PingText->SetText(FText::FromString(PingText));

			float FPS = 1.f / GetWorld()->GetDeltaSeconds();
			const FString FPSText = FString::Printf(TEXT("FPS : %d"), FMath::RoundToInt32(FPS));
			BlasterHUD->CharacterOverlay->FPSText->SetText(FText::FromString(FPSText));
		}
	}
}

// 检查玩家是否处于高延迟状态
void ABlasterPlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if (HighPingRunningTime >= CheckPingFrequency)
	{
		if (PlayerState)
		{
			// 检查玩家是否处于高延迟状态
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
			}
			if (PlayerState->GetPingInMilliseconds() > 40.f && PlayerState->GetPingInMilliseconds() < HighPingThreshold)
			{
				ServerReportPingStatus(false);
				// UE_LOG(LogTemp, Warning, TEXT("时间%s - 需要延迟补偿%f"), *FDateTime::UtcNow().ToString(), PlayerState->GetPingInMilliseconds());
			}
			else
			{
				ServerReportPingStatus(true);
				// UE_LOG(LogTemp, Warning, TEXT("时间%s - 不需要延迟补偿%f"), *FDateTime::UtcNow().ToString(), PlayerState->GetPingInMilliseconds());
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimaionPlaying = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimaionPlaying)
	{
		PingAnimationRunningTime += DeltaSeconds;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
			BlasterHUD->CharacterOverlay->PingText->SetRenderOpacity(1.f);
		}
	}
}

// 服务器报告Ping状态
void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHightPing)
{
	HighPingDelegate.Broadcast(bHightPing);
}

// 高延迟警告
void ABlasterPlayerController::HighPingWarning()
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->PingText;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetRenderOpacity(1.f);
		BlasterHUD->CharacterOverlay->PingText->SetRenderOpacity(0.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(
			BlasterHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

// 停止高延迟警告
void ABlasterPlayerController::StopHighPingWarning()
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->PingText;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetRenderOpacity(0.f);
		BlasterHUD->CharacterOverlay->PingText->SetRenderOpacity(1.f);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

// 服务器检查比赛状态
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;

	if (BlasterGameMode)
	{
		MatchState = BlasterGameMode->GetMatchState();
		WarmupTime = BlasterGameMode->WarmupTime;
		MatchTime = BlasterGameMode->MatchTime;
		CooldownTime = BlasterGameMode->CooldownTime;
		LevelStartingTime = BlasterGameMode->LevelStartingTime;
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

// 客户端加入比赛回调函数
void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	OnMatchStateSet(MatchState);

	// 等待开始
	if (MatchState == MatchState::WaitingToStart)
	{
		// 判断HUD是否为nullptr
		if (BlasterHUD)
		{
			if (BlasterHUD->Announcement)
			{
				BlasterHUD->Announcement->RemoveFromParent();
			}
			BlasterHUD->AddAnnouncement();
			// 隐藏公告
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		// 判断HUD是否为nullptr
		if (BlasterHUD)
		{
			if (BlasterHUD->Announcement)
			{
				BlasterHUD->Announcement->RemoveFromParent();
			}
			BlasterHUD->AddAnnouncement();
			// 隐藏公告
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	// GEngine->AddOnScreenDebugMessage(-1, 30, HasAuthority() ? FColor::Red : FColor::Yellow, FString::Printf(TEXT("%s的关卡开始时间 %f"), *GetName(), LevelStartingTime));
}

// 服务端时间同步回调函数
void ABlasterPlayerController::ServerRequestServetTime_Implementation(const float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientRequestServetTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

// 客户端时间同步回调函数
void ABlasterPlayerController::ClientRequestServetTime_Implementation(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

// 获取服务器时间
float ABlasterPlayerController::GetServerTime() const
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

// 玩家 possession 后调用
void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServetTime(GetWorld()->GetTimeSeconds());
	}
}

/**
 * 在游戏界面中设置HUD的健康值。
 * 
 * 此函数负责更新玩家控制器中的健康值显示。它首先确保BlasterHUD（游戏的头部上叠层）被正确初始化，
 * 然后根据提供的健康值和最大健康值更新HUD中的健康条和健康文本。
 * 
 * @param Health 当前健康值。这个值用于计算健康条的百分比和更新健康文本。
 * @param MaxHealth 最大健康值。这个值用于计算健康条的百分比和更新健康文本。
 */
void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	// 如果BlasterHUD及其相关元素已正确初始化，则更新健康值显示
	if (bHUDValid)
	{
		// 计算健康百分比并更新健康条
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		// 将当前健康值和最大健康值格式化为文本并更新健康文本
		const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

// 设置护盾
void ABlasterPlayerController::SetHUDShield(const float Shield, const float MaxShield)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;

	// 如果BlasterHUD及其相关元素已正确初始化，则更新护盾值显示
	if (bHUDValid)
	{
		// 计算健康百分比并更新护盾条
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		// 将当前护盾值和最护盾值格式化为文本并更新护盾文本
		const FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

// 设置分数
void ABlasterPlayerController::SetHUDScore(const float Score)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

// 隐藏队伍分数
void ABlasterPlayerController::HideTeamScores()
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText &&
		BlasterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
	}
}

// 初始化队伍分数
void ABlasterPlayerController::InitTeamScores() // 初始化队伍分数
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText &&
		BlasterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
	}
}

// 设置HUD红队分数
void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

// 设置HUD蓝队分数
void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

// 设置死亡
void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeat = true;
		HUDDefeats = Defeats;
	}
}

// 设置弹药
void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 Ammo)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

// 设置携带弹药
void ABlasterPlayerController::SetHUDCarriedAmmo(const int32 Ammo)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		// 分钟
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		// 秒
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		// 分钟
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		// 秒
		const int32 Seconds = CountdownTime - Minutes * 60;

		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

// 设置手雷
void ABlasterPlayerController::SetHUDGrenades(const int32 Grenades)
{
	// 检查BlasterHUD是否为空，如果为空则重新获取一个
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// 检查BlasterHUD及其相关元素是否已正确初始化
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		const FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenade = true;
		HUDGrenades = Grenades;
	}
}

// 设置匹配状态
void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamsMate)
{
	MatchState = State;
	bShowTeamScores = bTeamsMate;
	SetMatchState();
}

// 回调设置匹配状态
void ABlasterPlayerController::OnRep_MatchState()
{
	SetMatchState();
}

// 设置匹配状态
void ABlasterPlayerController::SetMatchState()
{
	// 匹配中
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	// 比赛结束冷却
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

// 匹配开始
void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		// 显示团队分数
		if (bShowTeamScores)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

// 冷却
void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();

		const bool bHUDValid = BlasterHUD->Announcement && BlasterHUD->Announcement->AnnouncementText && BlasterHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState)
			{
				// 获取当前比赛排名的玩家
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	// 尝试将当前控制的游戏角色转换为ABlasterCharacter类型，以便禁用其游戏玩法功能
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombat())
	{
		// 禁用角色的游戏玩法功能，通常用于游戏暂停或菜单界面显示时
		BlasterCharacter->bDisableGameplay = true;

		// 获取角色的战斗组件并设置开火按钮状态为未按下，防止在游戏玩法禁用时角色继续开火
		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}
}


// 获取信息文本
FString ABlasterPlayerController::GetInfoText(const TArray<class ABlasterPlayerState*>& Players)
{
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState == nullptr)return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::TherIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == BlasterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("获胜者：\n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

// 获取团队信息文本
FString ABlasterPlayerController::GetTeamsInfoText(class ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr) return FString();
	// 文本信息字符串
	FString InfoTextString;

	int32 RedTeamScore = BlasterGameState->RedTeamScore;
	int32 BlueTeamScore = BlasterGameState->BlueTeamScore;
	// 如果红蓝队伍得分都为0，则没有赢家
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::TherIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore) // 如果红蓝队伍得分相等，则队伍平局
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore) // 如果红队得分大于蓝队，则红队获胜   反者相反
	{
		InfoTextString = FString::Printf(TEXT("%s获胜了"), *Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s分数: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s获胜了"), *Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s分数: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}

	return InfoTextString;
}
