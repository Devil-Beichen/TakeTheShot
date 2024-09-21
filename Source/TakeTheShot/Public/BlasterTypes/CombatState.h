#pragma once

// 战斗状态
UENUM(BlueprintType)
enum class ECombatState:uint8
{
	// 无状态
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),

	// 换弹
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
