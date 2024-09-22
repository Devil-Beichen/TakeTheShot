// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"


class UTextBlock;

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
	class UProgressBar* HealthBar;

	// 当前血量文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	// 分数
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreAmount;

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
};
