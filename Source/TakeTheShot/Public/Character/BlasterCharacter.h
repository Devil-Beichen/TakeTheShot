// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;


/*	爆破角色的基类
 * 
 */

UCLASS()
class TAKETHESHOT_API ABlasterCharacter : public ACharacter
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

protected:
	virtual void BeginPlay() override;

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
	 *  服务器端处理慢走按钮松开事件
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
};
