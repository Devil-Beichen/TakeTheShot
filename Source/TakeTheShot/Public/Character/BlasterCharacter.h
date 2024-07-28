// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

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

	// 绑定输入
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BindInput();

	/**
	 * 当AI控制器占有此角色时被调用。
	 * 
	 * 此函数被设计为被AI控制器继承和重写，以处理特定于控制器的占有逻辑。
	 * 当一个新的控制器占有这个角色时，这个函数将被调用，允许新的控制器对角色进行初始化设置。
	 * 
	 * @param NewController 占有此角色的新控制器。它是一个AController类的指针，可以是任何继承自AController的类实例。
	 */
	// virtual void PossessedBy(AController* NewController) override;

protected:
	virtual void BeginPlay() override;

private:
	// 相机摇臂
	UPROPERTY(VisibleAnywhere, Category="Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	// 相机
	UPROPERTY(VisibleAnywhere, Category="Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

public:
	//输入映射
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
};
