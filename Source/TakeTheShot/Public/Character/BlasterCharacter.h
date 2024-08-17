// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "BlasterComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Interface/InteractWithCrosshairsInterface.h"
#include "Types/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UAnimMontage;


/*	爆破角色的基类
 * 
 */

UCLASS()
class TAKETHESHOT_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;

	// 添加默认映射上下文
	UFUNCTION()
	void AddDefaultMappingContext();

	// 移除映射上下文
	UFUNCTION()
	void RemoveMappingContext() const;

	/**	播放射击蒙太奇
	 * 
	 * @param bAiming 瞄准
	 */
	void PlayFireMontage(const bool bAiming) const;

	// 多播放命中重播蒙太奇
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitReactMontage();

protected:
	virtual void BeginPlay() override;

	/**	瞄准偏移
	 * 
	 * @param DeltaTime 帧时间
	 */
	void AimOffset(const float DeltaTime);

#pragma region 按键输入相关

	//输入映射
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext = nullptr;

	// 定义用于跳跃操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction = nullptr;

	// 定义用于移动操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction = nullptr;

	// 定义用于查看操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction = nullptr;

	// 定义用于蹲下操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction = nullptr;

	// 定义用于装备操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction = nullptr;

	// 定义一个UInputAction类型的动作，用于处理减速操作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlowAction = nullptr;

	// 定义一个UInputAction类型的动作，用于处理瞄准操作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimingAction = nullptr;

	// 定义一个UInputAction类型的动作，用于处理射击操作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction = nullptr;

	// 增强输入子系统
	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* EnhancedSubsystem = nullptr;

	/**
	 * 控制角色移动。
	 * 
	 * @param Value 输入动作的值，包含移动的方向和力度等信息。
	 */
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	/**
	 * 控制角色视角转动。
	 * 
	 * @param Value 输入动作的值，包含视角转动的方向和速度等信息。
	 */
	UFUNCTION()
	void Look(const FInputActionValue& Value);

	/**
	 * 触发角色跳跃动作。
	 */
	UFUNCTION()
	void Jump_Started();

	/**
	 * 停止跳跃动作，让角色开始下落。
	 */
	UFUNCTION()
	void Jump_Completed();

	/**
	 * 触发角色蹲下动作。
	 */
	UFUNCTION()
	void Crouch_Started();

	/**
	 *  按下慢走按钮
	 */
	UFUNCTION()
	void Slow_Started();

	/**
	 *  服务器端处理慢走按下事件
	 */
	UFUNCTION(Server, Reliable)
	void ServerSlowStarted();

	// 多播处理慢走按钮按下事件
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSlowStarted();

	/**
	 * 拾取武器按下动作
	 */
	UFUNCTION()
	void Equip_Started();

	// 瞄准持续按下事件
	UFUNCTION()
	void Aiming_Triggered();

	/**
	 * 瞄准持续松开事件
	 */
	UFUNCTION()
	void Aiming_Completed();

	/**
	 * 射击按下动作
	 */
	UFUNCTION()
	void Fire_Started();

	/**
	 * 射击松开动作
	 */
	UFUNCTION()
	void Fire_Completed();


#pragma endregion

#pragma  region 组件相关

private:
	// 相机摇臂
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	// 相机
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

public:
	/**
	 * 获取相机Boom组件。
	 * 
	 * 该函数返回一个指向USpringArmComponent的指针，该组件用于控制相机的位置和方向。
	 * 通过这个函数，可以方便地访问到相机Boom，进而对相机的位置进行调整和控制。
	 * 
	 * @return 返回一个USpringArmComponent类型的指针，指向相机Boom组件。
	 */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/**
	 * 获取跟随相机组件。
	 * 
	 * 该函数返回一个指向UCameraComponent的指针，该组件用于实际的相机视图渲染。
	 * 通过这个函数，可以方便地访问到跟随相机，进而对相机的视图进行调整和控制。
	 * 
	 * @return 返回一个UCameraComponent类型的指针，指向跟随相机组件。
	 */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma endregion

#pragma region UMG相关

private:
	// 在头顶上的UMG组件
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget = nullptr;

#pragma endregion

#pragma  region 角色武器相关

public:
	// 玩家是否处于慢速状态
	UPROPERTY()
	bool bIsSlowWalk = false;

	// 慢走速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move, meta=(AllowPrivateAccess = "true"))
	float SlowWalkSpeed = 100.f;

	// 跑步速度
	float RunSpeed = 350.f;

private:
	/**
	 * 定义一个AWeapon类的指针成员变量，用于存储当前重叠的武器对象
	 * 通过ReplicatedUsing属性指定复制时使用OnRep_OverlappingWeapon函数进行处理
	 */
	UPROPERTY(ReplicatedUsing="OnRep_OverlappingWeapon")
	class AWeapon* OverlappingWeapon = nullptr;

	/**
	 * 当玩家与武器重叠时的处理函数
	 * 
	 * 该函数在玩家与武器重叠时被调用，用于处理相关的逻辑
	 * 
	 */
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const;

	/**	使用VisibleAnywhere宏声明一个全局可见的属性
	*	这里声明了一个指向CombatComponent的指针，用于管理游戏中的战斗相关逻辑
	*/
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat = nullptr;

	/*  服务器端可靠地处理装备按钮按下事件
	*	该函数通过可靠的消息传输在服务器上触发，确保玩家角色能够正确地进行装备操作
	*/
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// 存储角色的左右角偏移量，表示角色在左右的偏移量。
	float AO_Yaw;
	// 左右偏移角度的插值目标值
	float InterpAO_Yaw;

	// 存储角色的上下角偏移量，表示角色在上下的偏移量。
	float AO_Pitch;

	// 存储角色的初始瞄准角度，表示角色的初始瞄准角度。
	FRotator StartingAimRotation;

	// 存储角色的转向状态，表示角色的转向状态。
	ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	// 原地转向函数
	void TurnInPlace(const float DeltaTime);

	// 角色的射击 Montage 动画
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	UAnimMontage* FireWeaponMontage = nullptr;

	// 角色受击 Montage 动画
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	UAnimMontage* HitReactMontage = nullptr;

	// 如果接近相机就隐藏角色
	void HideCameraIfCharacterClose();

	// 相机距离阈值
	UPROPERTY(EditDefaultsOnly, Category="Camera")
	float CameraThreshold = 200.f;

public:
	/**
	 * 设置当前重叠的武器对象
	 * 
	 * 此函数用于在角色或其他游戏实体中设置当前重叠的武器对象它用于处理武器拾取或重叠检测的逻辑
	 * 
	 * @param Weapon 要设置的重叠武器对象指针，可以为nullptr
	 */
	void SetOverlappingWeapon(AWeapon* Weapon);

	/**
	 * 检查角色是否正在装备武器
	 * 
	 * 该函数用于检查角色是否正在装备武器，返回一个布尔值表示是否装备了武器
	 * 
	 * @return 如果角色正在装备武器，则返回true，否则返回false
	 */
	bool IsWeaponEquipped() const;

	// 检查角色是否正在瞄准
	bool IsAiming() const;
#pragma  endregion

public:
	// 获取AO_Yaw的值
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }

	// 获取AO_Pitch的值
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }

	// 获取当前的武器对象
	AWeapon* GetEquippedWeapon() const;

	// 获取当前的角色的转向状态
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	// 获取命中点
	FORCEINLINE FVector GetHitTarget() const
	{
		if (Combat == nullptr) return FVector();
		return Combat->HitTarget;
	};
};
