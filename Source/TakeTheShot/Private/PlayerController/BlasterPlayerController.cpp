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
#include "GameState/BlasterGameState.h"
#include "PlayerState/BlasterPlayerState.h"

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

	SetHUDTime();

	CheckTimeSync(DeltaSeconds);
	PollInit();
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

	GEngine->AddOnScreenDebugMessage(2, 0.1, HasAuthority() ? FColor::Blue : FColor::Green,
	                                 FString::Printf(TEXT("预热时间 %f \n 服务器时间 %f \n 关卡开始时间%f \n 倒计时时间%f"),
	                                                 WarmupTime, GetServerTime(), LevelStartingTime, TimeLeft));

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


void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if (TimeSyncRunningTime >= TimeSyncFrequency && IsLocalController())
	{
		ServerRequestServetTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

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

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	OnMatchStateSet(MatchState);
	GEngine->AddOnScreenDebugMessage(-1, 30, HasAuthority() ? FColor::Red : FColor::Yellow, FString::Printf(TEXT("%s的关卡开始时间 %f"), *GetName(), LevelStartingTime));
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
	const float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime() const
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

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
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

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
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

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
		bInitializeCharacterOverlay = true;
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
}

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

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	SetMatchState();
}

void ABlasterPlayerController::OnRep_MatchState()
{
	SetMatchState();
}

void ABlasterPlayerController::SetMatchState()
{
	// 等待开始
	if (MatchState == MatchState::WaitingToStart)
	{
		if (BlasterHUD)
		{
			if (BlasterHUD->Announcement)
			{
				BlasterHUD->Announcement->RemoveFromParent();
			}
			BlasterHUD->AddAnnouncement();
		}
	}
	// 匹配中
	else if (MatchState == MatchState::InProgress)
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
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		BlasterHUD->AddCharacterOverlay();
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
			FString AnnouncementText = FString::Printf(TEXT("比赛结束了"));
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (BlasterGameState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString::Printf(TEXT("没有赢家！！！"));
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					InfoTextString = FString::Printf(TEXT("恭喜你，获得了胜利"));
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("获胜者：\n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString::Printf(TEXT("获胜的玩家有：\n"));
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
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
