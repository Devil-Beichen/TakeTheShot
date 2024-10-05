// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

/**
 *	热身公告
 */
UCLASS()
class TAKETHESHOT_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	// 热身时间
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WarmupTime;

	// 公告文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* AnnouncementText;

	// 提示文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* InfoText;
};
