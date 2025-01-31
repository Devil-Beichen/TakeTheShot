#pragma once

// 追踪长度
#define TRACE_LENGTH 80000.f

// 自定义深度 - 交互物品
#define CUSTOM_DEPTH_INTERITEM 1
// 自定义深度 - 拾取物品
#define CUSTOM_DEPTH_PICKUPITEMS 2
// 自定义深度 - 副武器
#define CUSTOM_DEPTH_SUBWEAPON 3

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
	// 旗
	EWT_Flag UMETA(DisplayName = "Flag"),
	// 默认最大
	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};
