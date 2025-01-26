// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "HUD/Announcement.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/ElimAnnouncement.h"

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
			DrawCrosshairs(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2d(0.f, 0.f), HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			DrawCrosshairs(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2d(-SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			DrawCrosshairs(HUDPackage.CrosshairsRight, ViewportCenter, FVector2d(SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			DrawCrosshairs(HUDPackage.CrosshairsTop, ViewportCenter, FVector2d(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			DrawCrosshairs(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2d(0.f, SpreadScaled), HUDPackage.CrosshairColor);
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

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * 添加角色叠加界面
 * 
 * 此函数用于创建并显示一个与玩家角色相关联的叠加界面，该界面通常包含角色相关信息的显示
 * 它是游戏HUD（Heads-Up Display）的一部分，用于向玩家提供必要的游戏内信息
 * 
 * 条件检查：
 * 1. CharacterOverlayClass 必须被正确设置，否则无法创建叠加界面
 * 2. 必须拥有一个有效的玩家控制器（PlayerController），因为叠加界面是通过玩家控制器创建的
 * 
 * 创建和显示叠加界面：
 * 如果以上条件满足，将使用玩家控制器和 CharacterOverlayClass 指定的类创建一个 UCharacterOverlay 对象
 * 创建成功后，通过将该对象添加到视口（Viewport），使其在游戏界面中显示出来
 * 
 * @note 如果 CharacterOverlayClass 未设置或玩家控制器不存在，函数将不执行任何操作
 */
void ABlasterHUD::AddCharacterOverlay()
{
	if (CharacterOverlay)
	{
		CharacterOverlay->RemoveFromRoot();
	}
	// 获取拥有此HUD的玩家控制器
	APlayerController* PlayerController = GetOwningPlayerController();

	// 检查CharacterOverlayClass是否被设置，以及是否拥有有效的玩家控制器
	if (CharacterOverlayClass && PlayerController)
	{
		// 创建UCharacterOverlay对象，并将其添加到玩家控制器的视口中
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

// 添加公告
void ABlasterHUD::AddAnnouncement()
{
	if (Announcement)
	{
		Announcement->RemoveFromRoot();
	}
	// 获取拥有此HUD的玩家控制器
	APlayerController* PlayerController = GetOwningPlayerController();

	// 检查CharacterOverlayClass是否被设置，以及是否拥有有效的玩家控制器
	if (AnnouncementClass && PlayerController)
	{
		// 创建UCharacterOverlay对象，并将其添加到玩家控制器的视口中
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

/**
* 添加淘汰公告板
* @param Attacker	击杀者
* @param Victim		被击杀者
*/
void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? Cast<APlayerController>(GetOwningPlayerController()) : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementClass)
	{
		if (UElimAnnouncement* ElimeAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass))
		{
			ElimeAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimeAnnouncementWidget->AddToViewport();

			// 遍历ElimMessages数组，调整每个消息的位置
			for (auto& Msg : ElimMessages)
			{
				// 检查消息对象和其公告框是否存在
				if (Msg && Msg->AnnouncementBox)
				{
					// 将消息的插槽转换为Canvas插槽
					if (UCanvasPanelSlot* CanvasSloy = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox))
					{
						// 获取当前插槽的位置
						FVector2D Position = CanvasSloy->GetPosition();
						// 计算新的位置，仅在Y轴上移动，移动距离为自身的高度
						FVector2D NewPosition(
							CanvasSloy->GetPosition().X,
							Position.Y - CanvasSloy->GetSize().Y
						);

						// 设置插槽的新位置
						CanvasSloy->SetPosition(NewPosition);
					}
				}
			}

			ElimMessages.Add(ElimeAnnouncementWidget);

			// 声明一个定时器句柄，用于后续管理定时器
			FTimerHandle ElimMsgTime;

			// 声明一个定时器委托，用于在定时器触发时调用指定函数
			FTimerDelegate ElimMsgDelegate;

			// 绑定一个UFunction到定时器委托中
			// 这里绑定了"ElimAnnouncementTimeFinished"函数，当定时器结束时，将调用此函数(可以传入参数)
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimeFinished"), ElimeAnnouncementWidget);

			// 使用世界定时器管理器设置一个定时器
			// ElimMsgTime: 定时器句柄，用于后续管理和取消定时器
			// ElimMsgDelegate: 定时器触发时调用的委托
			// ElimAnnouncementTime: 定时器触发前的延迟时间
			// false: 表示定时器只触发一次
			GetWorldTimerManager().SetTimer(
				ElimMsgTime,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false);
		}
	}
}

// 淘汰公告板时间结束
void ABlasterHUD::ElimAnnouncementTimeFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}
