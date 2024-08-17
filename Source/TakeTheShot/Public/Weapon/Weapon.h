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

class UAnimationAsset;
class UTexture2D;

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 设置是否激活PickupWidget。
	 * 
	 * 本函数用于控制PickupWidget的激活状态。当参数bActive为true时，表示激活PickupWidget；
	 * 当参数为false时，表示关闭PickupWidget。此设置可能用于控制在某些情况下是否显示或启用PickupWidget。
	 * 
	 * @param bShowWidget 指示PickupWidget是否应被激活的布尔值。true表示激活，false表示关闭。
	 */
	void ShowPickupWidget(const bool bShowWidget) const;

	/**
	 * 向指定的目标位置发射火球	是可以重写的。
	 * 
	 * 本函数实现了一个向目标位置发射火球的攻击动作，是常量成员函数
	 * 
	 * @param HitTarget 目标位置的向量表示，类型为FVector
	 */
	virtual void Fire(const FVector& HitTarget) const;

	/**
 *	武器准星的纹理资源
 */

	// 准星中心
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsCenter;

	// 准星左
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsLeft;

	// 准星右
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsRight;

	// 准星上
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsTop;

	// 准星下
	UPROPERTY(EditDefaultsOnly, Category="Crosshairs")
	UTexture2D* CrosshairsBottom;

	/**
	 *	瞄准时放大视野
	 */

	// 瞄准时放大视野
	UPROPERTY(EditDefaultsOnly)
	float ZoomedFOV = 30.f;

	// 瞄准时放大视野的插值速度
	UPROPERTY(EditDefaultsOnly)
	float ZoomInterpSpeed = 20.f;

	// 是自动武器
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	bool bAutomatic = false;

	// 开火间隔
	UPROPERTY(EditDefaultsOnly, Category=Combat, meta=(EditCondition = "bAutomatic == true", EditConditionHides))
	float FireDelay = 0.15f;

protected:
	virtual void BeginPlay() override;

	/**
	 * 当角色的碰撞球体与另一个演员的碰撞体发生重叠时，此函数将被调用。
	 * 
	 * @param OverlappedComponent 角色的碰撞球体组件，表示触发重叠的组件。
	 * @param OtherActor 发生重叠的另一个演员。
	 * @param OtherComp 另一个演员的碰撞组件。
	 * @param OtherBodyIndex 另一个碰撞体的索引。
	 * @param bFromSweep 如果重叠是由于扫动检测引起的，则此参数为true；否则为false。
	 * @param SweepResult 扫动检测的结果，包含重叠的详细信息。
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/**
	 * 当球体组件结束与其他组件重叠时触发的事件
	 * 
	 * @param OverlappedComponent 触发重叠事件的球体组件
	 * @param OtherActor 结束重叠的另一个演员（Actor）
	 * @param OtherComp 结束重叠的另一个演员的组件
	 * @param OtherBodyIndex 结束重叠的另一个身体（如果适用）的索引
	 */
	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

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
	UPROPERTY(ReplicatedUsing="OnRep_WeaponState", VisibleAnywhere, Category="Weapon Properties")
	EWeaponState WeaponState = EWeaponState::EWS_Initial;

	// 当武器状态发生变化时回调函数 只会在客户端执行
	UFUNCTION()
	void OnRep_WeaponState();

	// 武器拾取组件，用于显示武器的拾取提示
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	class UWidgetComponent* PickupWidget = nullptr;

	// 武器射击动画
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UAnimationAsset* FireAnimation = nullptr;

	// 子弹壳类
	UPROPERTY(EditDefaultsOnly, Category="Weapon Properties")
	TSubclassOf<class ACasing> CasingClass;

public:
	// 设置武器状态
	void SetWeaponState(const EWeaponState State);

	// 获取球形组件
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	// 获取武器模型组件
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	// 获取瞄准时放大视野
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	// 获取瞄准时放大视野的插值速度
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
};
