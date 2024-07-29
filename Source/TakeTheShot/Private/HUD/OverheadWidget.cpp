// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/TextBlock.h"

/**
 * 设置头顶Widget的显示文本。
 * 
 * @param TextToDisplay 指向要显示的文本的字符串指针。如果为nullptr，则不进行任何操作。
 */
void UOverheadWidget::SetDisplayText(const FString* const TextToDisplay) const
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(*TextToDisplay));
	}
}

/**
 * 根据玩家的网络角色显示头顶Widget的文本。
 * 
 * @param InPawn 指向要显示网络角色的玩家角色的指针。
 */
void UOverheadWidget::ShowPlayerNetRole(const APawn* const InPawn) const
{
	// 获取玩家的网络角色
	const ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;

	// 根据不同的网络角色设置字符串
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority: // 对演员的权威控制
		Role = FString(TEXT("Authority"));
		break;
	case ENetRole::ROLE_AutonomousProxy: // 本地自治代理
		Role = FString(TEXT("Autonomous Proxy"));
		break;
	case ENetRole::ROLE_SimulatedProxy: //  模拟代理
		Role = FString(TEXT("Simulated Proxy"));
		break;
	case ENetRole::ROLE_None:
		Role = FString(TEXT("None"));
		break;
	case ENetRole::ROLE_MAX:
		Role = FString(TEXT("MAX"));
		break;
	}

	// 格式化显示的文本为"Local Role: 角色名称"
	const FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	SetDisplayText(&LocalRoleString);
}

/**
 * 控件的原生析构函数。
 * 
 * 从父控件中移除自身并调用超类的析构函数。
 */
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
