#pragma once

// 追踪长度
#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType:uint8
{
	// 突击步枪
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	// 火箭弹发射器
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	// 手枪
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	// 冲锋枪
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	// 霰弹枪
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	// 狙击枪
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	// 榴弹发射器
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	// 默认最大
	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};
