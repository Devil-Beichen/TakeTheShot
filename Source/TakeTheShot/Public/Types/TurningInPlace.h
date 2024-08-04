#pragma once

// 定义转向意向枚举
UENUM(BlueprintType)
enum class ETurningInPlace :uint8
{
	// 左转状态
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	// 右转状态
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	// 未转向状态
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),
	// 最大值状态，用于表示默认或无效的转向状态
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};
