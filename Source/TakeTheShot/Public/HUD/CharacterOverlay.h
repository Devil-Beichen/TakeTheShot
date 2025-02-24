﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"


class UTextBlock;
class UProgressBar;
class UImage;

/**
 * 角色属性的UI
 */
UCLASS()
class TAKETHESHOT_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	// 血条
	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthBar;

	// 当前血量文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	// 护盾
	UPROPERTY(meta=(BindWidget))
	UProgressBar* ShieldBar;

	// 当前护盾文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShieldText;

	// 分数
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreAmount;

	// 蓝队分数
	UPROPERTY(meta=(BindWidget))
	UTextBlock* BlueTeamScore;

	// 团队分数分界线
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreSpacerText;

	// 红队分数
	UPROPERTY(meta=(BindWidget))
	UTextBlock* RedTeamScore;

	// 死亡次数
	UPROPERTY(meta=(BindWidget))
	UTextBlock* DefeatsAmount;

	// 弹匣容量
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponAmmoAmount;

	// 携带弹药量(备弹量)
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CarriedAmmoAmount;

	// 倒计时
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MatchCountdownText;

	// 手雷数量
	UPROPERTY(meta=(BindWidget))
	UTextBlock* GrenadesText;

	// 高延迟图形
	UPROPERTY(meta=(BindWidget))
	UImage* HighPingImage;

	// 高延迟动画
	UPROPERTY(meta=(BindWidgetAnim), Transient) // Transient 属性是暂态的，这意味着它不会被保存或加载。以这种方式标记的属性将在加载时被零填充。
	UWidgetAnimation* HighPingAnimation;

	// 延迟
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PingText;

	// FPS
	UPROPERTY(meta=(BindWidget))
	UTextBlock* FPSText;
};
