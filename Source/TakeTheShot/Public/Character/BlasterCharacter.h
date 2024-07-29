// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	UInputMappingContext* DefaultMappingContext;

	// 定义用于跳跃操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	// 定义用于移动操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// 定义用于查看操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// 定义用于蹲下操作的输入动作，允许在编辑器中任何地方进行编辑和绑定，类别为Input，允许私有访问
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

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
};
