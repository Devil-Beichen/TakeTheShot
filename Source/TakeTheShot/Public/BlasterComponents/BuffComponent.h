// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

/**
 * buff组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TAKETHESHOT_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();

	/* 定义一个友元类ABlasterCharacter
	友元类可以访问此函数所属类的私有和保护成员
	这里声明友元类是为了在ABlasterCharacter类中访问当前类的某些成员或函数
	*/
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	// 定义一个指向Blaster角色的指针，用于在装备系统中引用角色实例
	UPROPERTY()
	ABlasterCharacter* Character = nullptr;

public:
};
