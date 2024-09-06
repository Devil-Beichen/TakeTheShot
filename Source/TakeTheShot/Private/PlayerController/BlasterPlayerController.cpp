// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"

#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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

// 玩家被控制的回调函数
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		BlasterCharacter->Initialize();
	}
}
