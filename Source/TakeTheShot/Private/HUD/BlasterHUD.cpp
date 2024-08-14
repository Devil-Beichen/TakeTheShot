// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();


	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;

		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			DrawCrosshairs(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2d(0.f, 0.f), FLinearColor::White);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			DrawCrosshairs(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2d(-SpreadScaled, 0.f), FLinearColor::White);
		}
		if (HUDPackage.CrosshairsRight)
		{
			DrawCrosshairs(HUDPackage.CrosshairsRight, ViewportCenter, FVector2d(SpreadScaled, 0.f), FLinearColor::White);
		}
		if (HUDPackage.CrosshairsTop)
		{
			DrawCrosshairs(HUDPackage.CrosshairsTop, ViewportCenter, FVector2d(0.f, -SpreadScaled), FLinearColor::White);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			DrawCrosshairs(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2d(0.f, SpreadScaled), FLinearColor::White);
		}
	}
}

void ABlasterHUD::DrawCrosshairs(UTexture2D* Texture, const FVector2D& ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	// 定义纹理的宽度，用于后续计算纹理的绘制位置和尺寸
	const float TextureWidth = Texture->GetSizeX();

	// 定义纹理的高度，用于后续计算纹理的绘制位置和尺寸
	const float TextureHeight = Texture->GetSizeY();

	// 计算纹理的绘制点，使其以视口中心为基准点进行绘制
	// 这里使用视口中心减去纹理的一半宽度（或高度）来确保纹理的中心与视口的中心对齐
	const FVector2d TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	// 绘制纹理
	// 这里提供了纹理对象以及纹理在屏幕上的起始绘制点和绘制尺寸，确保纹理按照预期显示
	// 参数依次为：要绘制的纹理对象，纹理的绘制起始点X坐标，纹理的绘制起始点Y坐标，
	// 纹理的绘制宽度，纹理的绘制高度，纹理的源起始点U坐标，纹理的源起始点V坐标，
	// 纹理的源宽度U坐标，纹理的源高度V坐标，绘制的颜色
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}
