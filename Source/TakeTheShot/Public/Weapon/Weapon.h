// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "Weapon.generated.h"

// 武器状态枚举
UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	//  装备的初始状态。指示装备是新获得的，尚未被装备。
	EWS_Initial UMETA(DisplayName = "Initial State"),
	//  装备已装备。指示装备已装备到角色身上。
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	// 装备到副武器
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"),
	//  装备已丢弃。指示装备已被丢弃。
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	// 装备穿戴状态的最大数量。这是一个占位符值，用来指示枚举的结束。
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

// 开火类型枚举
UENUM(BlueprintType)
enum class EFireType : uint8
{
	// 命中扫描
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	// 发射子弹
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	// 霰弹
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 重写OnRep_Owner函数
	virtual void OnRep_Owner() override;

	// 设置HUD的弹药显示
	void SetHUDAmmo();

	/**
	 * 设置是否激活PickupWidget。
	 * 
	 * 本函数用于控制PickupWidget的激活状态。当参数bActive为true时，表示激活PickupWidget；
	 * 当参数为false时，表示关闭PickupWidget。此设置可能用于控制在某些情况下是否显示或启用PickupWidget。
	 * 
	 * @param bShowWidget 指示PickupWidget是否应被激活的布尔值。true表示激活，false表示关闭。
	 */
	void ShowPickupWidget(const bool bShowWidget);

	/**
	 * 向指定的目标位置发射火球	是可以重写的。
	 * 
	 * 本函数实现了一个向目标位置发射火球的攻击动作，是常量成员函数
	 * 
	 * @param HitTargets 命中点数组
	 */
	virtual void Fire(const TArray<FVector_NetQuantize>& HitTargets);

	/**
	* 随机散射命中点
	* @param HitTarget		命中点
	* @param HitTargets		命中点数组
	* @return 
	*/
	void TraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

	// 丢弃武器
	void Dropped();

	// 添加子弹
	void AddAmmo(int AmmoToAdd);

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
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	float FireDelay = 0.15f;

	// 拾取武器的音效
	UPROPERTY(EditDefaultsOnly)
	class USoundCue* EquipSound;

	// 开启自定义深度
	void EnableCustomDepth(const bool bEnable);

	// 删除武器
	bool bDestroyWeapon = false;

	// 武器开火类型
	EFireType FireType;

	// 是否使用散射
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	bool bUseScatter = false;

	// 弹丸数量
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	uint32 NumberOfPellets = 1;

	/**
	*  带散点的命中追踪轨迹结束
	*/

	// 命中追踪轨迹结束
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	float DistanceToShere = 800.f;

	// 命中球体半径
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	float SphereRadius = 75.f;

protected:
	virtual void BeginPlay() override;

	// 武器状态设置
	virtual void OnWeaponStateSet();
	// 武器装备
	virtual void OnEquipped();

	// 装备副武器
	virtual void OnEquippedSecondary();

	// 武器丢弃
	virtual void OnDropped();

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

	// 当前子弹
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing="OnRep_Ammo")
	int32 Ammo = 30;

	// 消耗子弹
	void SpendRound();

	// 当剩余子弹数量发生变化时回调函数 只会在客户端执行
	UFUNCTION()
	void OnRep_Ammo();

	// 弹匣容量(备弹量)
	UPROPERTY(EditDefaultsOnly)
	int32 MagCapacity = 30;

	// 拥有自己的玩家角色
	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	// 拥有自己的玩家控制器
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	// 武器类型
	UPROPERTY(EditDefaultsOnly)
	EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle;

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
	// 子弹为空
	FORCEINLINE bool IsAmmoEmpty() const { return Ammo <= 0; }
	// 弹匣已满
	FORCEINLINE bool IsFull() const { return Ammo == MagCapacity; }
	// 获取武器类型
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	// 获取剩余子弹
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	// 获取弹匣容量
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
