#pragma once

UENUM(BlueprintType)
enum class EWeaponType:uint8
{
	// 突击步枪
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	// 默认最大
	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};
