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

	// 准星扩散
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	float CrosshairSpread = 0.f;
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

	/**
	 * 绘制十字准星
	 *
	 * 本函数用于在屏幕上绘制一个带有纹理的十字准星，准星的位置和颜色可以通过参数进行定制。
	 * 
	 * @param Texture 十字准星的纹理图片，用于定义十字准星的外观。
	 * @param ViewportCenter 视口中心点的坐标，用于确定十字准星的位置。
	 * @param Spread 十字准星的扩展距离，用于调整十字准星的大小和形状。
	 * @param CrosshairColor 十字准星的颜色，用于定制十字准星的颜色外观。
	 */
	void DrawCrosshairs(UTexture2D* Texture, const FVector2D& ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	// 十字准星最大扩展距离
	UPROPERTY(EditAnywhere, Category="Crosshairs")
	float CrosshairSpreadMax = 16.0f;

public:
	// 设置HUD的资源包
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
