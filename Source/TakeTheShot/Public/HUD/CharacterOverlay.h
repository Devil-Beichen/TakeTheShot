﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

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
	class UTextBlock* HealthText;
};