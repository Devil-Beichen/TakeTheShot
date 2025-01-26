// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

/**
 * 淘汰通告
 */
UCLASS()
class TAKETHESHOT_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 设置淘汰通告文本
	 * @param AttackerName	发起攻击的名字	
	 * @param VictimName	被攻击的名字
	 */
	void SetElimAnnouncementText(FString AttackerName, FString VictimName);

	// 淘汰通告文本框
	UPROPERTY(meta=(BindWidget))
	class UHorizontalBox* AnnouncementBox;

	// 淘汰通告文本
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* AnnouncementText;
};
