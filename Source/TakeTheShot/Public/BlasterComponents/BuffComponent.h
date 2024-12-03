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

	/**
	 * 恢复生命
	 * @param HealAmount	恢复的总生命值 
	 * @param HealingTime	恢复持续的时间
	 */
	void Heal(float HealAmount, float HealingTime);

protected:
	virtual void BeginPlay() override;

	// 治疗增加
	void HealRampUp(float DetaTime);

private:
	// 定义一个指向Blaster角色的指针，用于在装备系统中引用角色实例
	UPROPERTY()
	ABlasterCharacter* Character = nullptr;

	// 用于记录是否正在恢复生命
	bool bHealing = false;
	// 目前恢复的速率
	float HealingRate = 0.f;
	// 还需要恢复的
	float AmountToHeal = 0.f;

public:
};
