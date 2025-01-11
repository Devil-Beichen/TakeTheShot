#pragma once

// 战斗状态
UENUM(BlueprintType)
enum class ECombatState:uint8
{
	// 无状态
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),

	// 换弹
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	// 投掷手雷
	ECS_ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"),

	// 切换武器
	ECS_SwappingWeapons UMETA(DisplayName = "SwappingWeapons"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
