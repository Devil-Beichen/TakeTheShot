#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

// 追踪长度
#define TRACE_LENGTH 80000.f

class ABlasterCharacter;
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

	// 服务器端开火
	UFUNCTION(Server, Reliable)
	void ServerFire() const;

	// 多播开火
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire() const;

	/**
	 * 根据十字准星的位置进行光线追踪，并更新命中结果。
	 * 
	 * 该函数主要用于游戏或其他需要光线追踪的场景中，通过十字准星的位置向场景发射一条光线，
	 * 检测是否击中场景中的物体，并将击中结果存储在传入的命中结果变量中。
	 * 
	 * @param TraceHitResult 一个引用类型的FHitResult对象，用于存储光线追踪的命中结果，包括命中的位置、法线、被命中的物体等信息。
	 */
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	// 定义一个指向Blaster角色的指针，用于在装备系统中引用角色实例
	UPROPERTY()
	ABlasterCharacter* Character = nullptr;

	// 定义一个指向当前装备武器的指针，用于在角色中引用和操作武器实例
	UPROPERTY(ReplicatedUsing="OnRep_EquippedWeapon")
	AWeapon* EquippedWeapon = nullptr;

	// 定义一个布尔变量，用于指示角色是否正在瞄准
	UPROPERTY(Replicated)
	bool bAiming = false;

	// 角色是否正在发射
	bool bFireButtonPressed = false;

	// 命中目标的位置
	FVector HitTargetLocation;

public:
};
