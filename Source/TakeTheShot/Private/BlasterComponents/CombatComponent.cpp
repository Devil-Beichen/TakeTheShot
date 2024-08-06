#include "BlasterComponents/CombatComponent.h"

#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	// 指定Aiming属性需要在服务器和客户端之间同步复制
	DOREPLIFETIME(UCombatComponent, bAiming);
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

	// 禁用角色的移动方向与旋转方向的自动对齐
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;

	// 设置角色是否使用控制器的偏航旋转
	Character->bUseControllerRotationYaw = true;
}

// 当装备的武器发生变化时调用此函数
void UCombatComponent::OnRep_EquippedWeapon() const
{
	// 此处应添加具体的逻辑代码，以响应装备武器的变化
	if (EquippedWeapon && Character)
	{
		// 禁用角色的移动方向与旋转方向的自动对齐
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;

		// 设置角色是否使用控制器的偏航旋转
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(const bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (Character && bFireButtonPressed)
	{
		Character->PlayFireMontage(bPressed);
	}
}

// 设置瞄准状态的函数
// @param bIsAiming：布尔值，表示是否处于瞄准状态
void UCombatComponent::SetAiming(const bool bIsAiming)
{
	// 更新当前是否处于瞄准状态
	bAiming = bIsAiming;

	if (Character->HasAuthority())
	{
		// 设置瞄准速度，根据当前的瞄准状态调整
		MulticastSetAimingSpeed();
	}
	// 向服务器请求设置瞄准状态，确保服务器和客户端状态同步
	ServerSetAiming(bIsAiming);
}

void UCombatComponent::MulticastSetAimingSpeed_Implementation() const
{
	// 根据是否瞄准状态调整角色移动速度
	if (bAiming)
	{
		// 如果正在瞄准，将角色的最大移动速度设置为慢走速度
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->SlowWalkSpeed;

		Character->bIsSlowWalk = true;
	}
	else
	{
		// 如果未在瞄准状态，将角色的最大移动速度设置为跑步速度
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->RunSpeed;

		Character->bIsSlowWalk = false;
	}
}

// 服务器端设置瞄准状态的函数，用于同步客户端和服务器端的状态
// @param bIsAiming：布尔值，表示是否处于瞄准状态
void UCombatComponent::ServerSetAiming_Implementation(const bool bIsAiming)
{
	bAiming = bIsAiming;
	MulticastSetAimingSpeed();
}
