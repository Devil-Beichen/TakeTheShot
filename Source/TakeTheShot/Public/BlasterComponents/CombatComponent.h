#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

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

	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

private:
	// 定义一个指向Blaster角色的指针，用于在装备系统中引用角色实例
	UPROPERTY()
	ABlasterCharacter* Character = nullptr;

	// 定义一个指向当前装备武器的指针，用于在角色中引用和操作武器实例
	UPROPERTY()
	AWeapon* EquippedWeapon = nullptr;

public:
};
