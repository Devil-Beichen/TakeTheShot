#include "BlasterComponents/CombatComponent.h"

#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// 获取此组件的生命周期内需要复制的属性列表
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 调用基类方法以获取基类的生命周期复制属性
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 指定EquippedWeapon属性需要在服务器和客户端之间同步复制
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
}

// 将武器装备到角色的右手
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	// 检查角色和待装备的武器是否为空，如果为空则不执行任何操作
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	// 将待装备的武器设置为当前装备的武器
	EquippedWeapon = WeaponToEquip;
	// 设置武器状态为已装备
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	// 尝试找到角色右手的插槽，如果找到，则将武器装备到该插槽
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	// 设置武器的所有者为角色
	EquippedWeapon->SetOwner(Character);
}
