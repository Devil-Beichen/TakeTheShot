// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	//  装备的初始状态。指示装备是新获得的，尚未被装备。
	EWS_Initial UMETA(DisplayName = "Initial State"),
	//  装备已装备。指示装备已装备到角色身上。
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	//  装备已丢弃。指示装备已被丢弃。
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	// 装备穿戴状态的最大数量。这是一个占位符值，用来指示枚举的结束。
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**	武器基类
 * 
 */
UCLASS()
class TAKETHESHOT_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	// 武器模型组件，用于可视化展示武器
	// 类型为USkeletalMeshComponent，允许在任何地方可见
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh = nullptr;

	// 武器影响区域组件，以球形定义影响范围
	// 类型为USphereComponent，允许在任何地方可见
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	class USphereComponent* AreaSphere = nullptr;

	// 武器状态变量，用于表示武器的当前状态
	// 取值自EWeaponState枚举，初始状态为EWS_Initial
	UPROPERTY(VisibleAnywhere)
	EWeaponState WeaponState = EWeaponState::EWS_Initial;

public:
};
