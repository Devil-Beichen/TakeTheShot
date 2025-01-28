#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	// 红队
	ET_RedTeam UMETA(DisplayName = "RedTeam"),
	// 蓝队
	ET_BlueTeam UMETA(DisplayName = "BlueTeam"),
	// 没有队伍
	ET_NoTeam UMETA(DisplayName = "NoTeam"),
	// 默认
	ET_MAX UMETA(DisplayName = "DefaultMAX")
};
