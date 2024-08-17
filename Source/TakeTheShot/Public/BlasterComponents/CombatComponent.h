#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"

// 追踪长度
#define TRACE_LENGTH 80000.f

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
	void OnRep_EquippedWeapon() const;

	/**
	* 处理发射按钮按下事件
	* 当发射按钮被按下或松开时，调用本函数通知对象
	* @param bPressed 指示按钮是否被按下当按钮被按下时，bPressed为true；当按钮被松开时，bPressed为false
	*/
	void FireButtonPressed(const bool bPressed);

	/**
	 * 服务器端发射函数
	 * 该函数通过网络可靠地发送一个射击动作，指定射击的目标点
	 * 
	 * @param TraceHitTarget 射击射中目标的位置，以网络量化向量表示
	 *                        该参数通过网络传输，确保在服务器和客户端之间同步射击目标
	 */
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget) const;

	/**
	 * 网络多播函数，用于可靠地发送射击指令
	 * 该函数通过网络将射击目标的位置通知给所有订阅此事件的客户端
	 * 
	 * @param TraceHitTarget 射击命中的目标位置，使用FVector_NetQuantize格式来优化网络传输
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget) const;

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
	UPROPERTY(ReplicatedUsing="OnRep_EquippedWeapon")
	AWeapon* EquippedWeapon = nullptr;

	// 定义一个布尔变量，用于指示角色是否正在瞄准
	UPROPERTY(Replicated)
	bool bAiming = false;

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

	// 开火定时器结束
	void FireTimerFinished();

public:
};
