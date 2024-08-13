// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UTexture2D;

// HUD的包
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	// 准星中心
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsCenter;

	// 准星左
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsLeft;

	// 准星右
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsRight;

	// 准星上
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsTop;

	// 准星下
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsBottom;
};

/** 爆破角色的HUD
 * 
 */
UCLASS()
class TAKETHESHOT_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	// 绘制HUD
	virtual void DrawHUD() override;

private:
	// HUD的资源包
	FHUDPackage HUDPackage;

public:
	// 设置HUD的资源包
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
