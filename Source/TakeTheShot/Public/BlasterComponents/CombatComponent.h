﻿#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Weapon/WeaponTypes.h"
#include "BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;
class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TAKETHESHOT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* 定义一个友元类ABlasterCharacter
	 友元类可以访问此函数所属类的私有和保护成员
	 这里声明友元类是为了在ABlasterCharacter类中访问当前类的某些成员或函数
	 */
	friend class ABlasterCharacter;

	// 拾取武器的函数 只在服务器上面执行
	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

	// 切换武器
	void SwapWeapons();

	// 重新装填
	void Reload();

	// 切换武器绑定
	void SwapAttachWeapon();

	// 切换完成
	void SwapFinish();

	/**
	* 处理发射按钮按下事件
	* 当发射按钮被按下或松开时，调用本函数通知对象
	* @param bPressed 指示按钮是否被按下当按钮被按下时，bPressed为true；当按钮被松开时，bPressed为false
	*/
	void FireButtonPressed(const bool bPressed);

	// 跳到霰弹枪的装填结束动画
	void JumpToShotgunEnd();

	// 投掷手雷完成
	void ThrowGrenadeFinished();

	// 丢弃装备的武器
	void DropEquippedWeapon();

	// 将Actor绑定到的右手上
	void AttachActorToRightHand(AActor* ActorToAttach);

	// 将Actor绑定到的左手上
	void AttachActorToLeftHand(AActor* ActorToAttach);

	// 将Flag绑定到左手上
	void AttachFlagToLeftHand(AWeapon* Flag);

	// 将Actor绑定到背包上
	void AttachActorToBackpack(AActor* ActorToAttach);

	// 播放装备武器的音效
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	// 重新装填空武器
	void ReloadEmptyWeapon();

	// 显示或隐藏手雷
	void ShowAttachedGrenade(bool bShowGrenade);

	// 装备主武器
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	// 装备副武器
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	// 装备旗子
	void EquipFlag(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

	// 设置瞄准状态
	void SetAiming(const bool bIsAiming);

	// 多播设置瞄准速度
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetAimingSpeed() const;

	// 服务器端设置瞄准状态
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool bIsAiming);

	// 当装备的武器发生变化时调用此函数
	UFUNCTION()
	void OnRep_EquippedWeapon();

	// 设置当前装备的武器的状态
	void SetEquippedWeaponState();

	// 副武器发生变化的时候
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	// 设置副武器的状态
	void SetSecondaryWeaponState();

	// 副武器状态
	void SecondaryWeaponStatus();

	// 设置旗子状态
	void SetFlagState();

	// 旗子状态
	void FlagState();

	/**
	 * 根据十字准星的位置进行光线追踪，并更新命中结果。
	 * 
	 * 该函数主要用于游戏或其他需要光线追踪的场景中，通过十字准星的位置向场景发射一条光线，
	 * 检测是否击中场景中的物体，并将击中结果存储在传入的命中结果变量中。
	 * 
	 * @param TraceHitResult 一个引用类型的FHitResult对象，用于存储光线追踪的命中结果，包括命中的位置、法线、被命中的物体等信息。
	 */
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	/**
	* 更新HUD上的十字准星
	* 
	* 该函数用于更新HUD上的十字准星，根据当前角色的视角和角色的移动状态，
	* 更新HUD上的十字准星以反映角色的移动状态和视角。
	* 
	 * @param DeltaTime 帧之间的时间差，用于计算十字准星的移动速度
	*/
	void SetHUDCrosshairs(const float DeltaTime);

	// 服务器重新装填
	UFUNCTION(Server, Reliable)
	void ServerReload();

	// 处理弹药加载
	UFUNCTION()
	void HandleReload();

	// 完成装填
	void FinishReload();

	// 处理霰弹枪的弹药加载
	void ShotgunShellReload();

	// 获取当前角色的弹药数量
	int32 AmountToReload();

	// 投掷手雷
	void ThrowGrenade();

	// 服务器投掷手雷
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	// 手雷类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AProjectile> GrenadeClass;

	// 投掷手雷设置
	void ThrowGrenadeSet();

	// 发射手雷
	void LaunchGrenade();

	// 服务器发射手雷
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

public:
	// 拾取弹药
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	// 拾取手雷
	void PickupGrenade(int AddGrenade);

	// 在本地重新装填
	bool bLocallyReloading = false;

private:
	// 定义一个指向Blaster角色的指针，用于在装备系统中引用角色实例
	UPROPERTY()
	ABlasterCharacter* Character = nullptr;

	// 定义一个指向Blaster玩家控制器的指针，用于在装备系统中引用玩家控制器实例
	UPROPERTY()
	ABlasterPlayerController* Controller = nullptr;

	// 定义一个指向HUD的指针，用于在装备系统中引用HUD实例
	UPROPERTY()
	class ABlasterHUD* HUD = nullptr;

	// 定义一个指向当前装备武器的指针，用于在角色中引用和操作武器实例
	UPROPERTY(ReplicatedUsing = "OnRep_EquippedWeapon")
	AWeapon* EquippedWeapon = nullptr;

	// 定义一个指向副武器的指针，用于在角色中引用和操作副武器实例
	UPROPERTY(ReplicatedUsing = "OnRep_SecondaryWeapon")
	AWeapon* SecondaryWeapon = nullptr;

	// 定义一个布尔变量，用于指示角色是否正在瞄准
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	// 瞄准按钮是否被按下
	bool bAimButtonPressed = false;

	// 瞄准回调
	UFUNCTION()
	void OnRep_Aiming();

	// 角色是否正在发射
	bool bFireButtonPressed = false;

	/**
	 *  HUD 和准星
	 */

	// 准星速度因子
	float CrosshairVelocityFactor = 0.f;

	// 在空中准星速度
	float CrosshairInAirFactor = 0.f;

	// 瞄准的系数
	float CrosshairAimFactor = 0.f;

	// 射击的系数
	float CrosshairShootingFactor = 0.f;

	FVector HitTarget; // 命中点

	// 初始化一个HUDPackage对象，用于存储十字准星的信息
	FHUDPackage HUDPackage;

	/**
	 *  瞄准的视角(FOV)
	 */

	// 不在瞄准的时候，默认的FOV
	float DefaultFOV = 0.f;

	// 瞄准时放大视野
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float ZoomedFOV = 30.f;

	// 当前FOV
	float CurrentFOV = 0.f;

	// 瞄准时放大视野的插值速度
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	// 用于处理视场角（FOV）的插值变化
	void InterpFOV(float DeltaTime);

	// 可以开火
	bool bCanFire = true;

	/**
	 *  自动开火定时器
	 */

	// 开火定时器
	FTimerHandle FireTimer;

	// 开火定时器开始
	void StartFireTimer();
	// 开火
	void Fire();

	// 武器开火
	void WeaponFire();

	/**
	* 服务器端发射函数
	* 该函数通过网络可靠地发送一个射击动作，指定射击的目标点
	* 
	* @param TraceHitTargets	射击射中目标的位置，以网络量化向量表示
	* @param FireDelay			射击的延迟时间，以秒为单位                 
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	/**
	 * 网络多播函数，用于可靠地发送射击指令
	 * 该函数通过网络将射击目标的位置通知给所有订阅此事件的客户端
	 * 
	 * @param TraceHitTargets 射击命中的目标位置，使用FVector_NetQuantize格式来优化网络传输
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	// 本地发射
	void LocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	// 开火定时器结束
	void FireTimerFinished();

	// 可以开火
	bool CanFire();

	// 当前装备武器携带的弹药
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo = 0;

	// 回调携带弹药发生改变(只会发生在客户端)
	UFUNCTION()
	void OnRep_CarriedAmmo();

	// 更新携带弹药
	void UpdateCarriedAmmo();

	// 携带弹药的哈希表
	TMap<EWeaponType, int32> CarriedAmmoMap;

	// 最大携带弹药
	UPROPERTY(EditDefaultsOnly)
	int32 MaxCarriedAmmo = 500;

	// 开始的步枪备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingARAmmo = 30;

	// 开始的火箭弹备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingRocketAmmo = 0;

	// 开始的手枪备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingPistolAmmo = 30;

	// 开始的冲锋枪备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingSMGAmmo = 60;

	// 开始的霰弹枪备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingShotgunAmmo = 20;

	// 开始的狙击枪备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingSniperAmmo = 20;

	// 开始的榴弹枪备用弹药
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	int32 StartingGrenadeLauncherAmmo = 5;

	// 初始化携带弹药
	void InitializeCarriedAmmo();

	// 战斗状态
	UPROPERTY(ReplicatedUsing="OnRep_CombatState")
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	// 当状态发生改变的时候触发回调函数（只会在客户端触发）
	UFUNCTION()
	void OnRep_CombatState();

	// 完成重新换弹
	void UpdateAmmoValues();

	// 更新霰弹枪的弹药值
	void UpdateShotgunAmmoValues();

	// 携带的手雷
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	// 回调手雷数量改变
	UFUNCTION()
	void OnRep_Grenades();

	// 最大携带的手雷
	UPROPERTY(EditDefaultsOnly)
	int32 MaxGrenades = 4;

	// 更新HUD上的手雷数量
	void UpdateHUDGrenades();

	// 是否持有旗子
	UPROPERTY(Replicated)
	bool bHoldingTheFlag = false;

	// 持有的旗子
	UPROPERTY(ReplicatedUsing = "OnRep_TheFlag")
	AWeapon* TheFlag = nullptr;

	// 回调持有旗子
	UFUNCTION()
	void OnRep_TheFlag();

public:
	// 获取携带的手雷
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	// 获取最大携带的手雷
	FORCEINLINE int32 GetMaxGrenades() const { return MaxGrenades; }

	// 是否应该切换武器
	bool ShouldSwapWeapons() const;
};
