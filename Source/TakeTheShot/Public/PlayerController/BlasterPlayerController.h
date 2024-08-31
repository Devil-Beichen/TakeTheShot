// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

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

	// 重写拥有pawn
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD = nullptr;
};
