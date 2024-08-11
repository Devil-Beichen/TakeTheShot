#include "BlasterComponents/CombatComponent.h"

#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
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

// 当开火按钮被按下时，处理相关逻辑
void UCombatComponent::FireButtonPressed(const bool bPressed)
{
	// 记录开火按钮的按下状态
	bFireButtonPressed = bPressed;

	// 如果没有装备武器，则不执行任何操作
	if (EquippedWeapon == nullptr) return;

	// 如果开火按钮被按下
	if (bFireButtonPressed)
	{
		// 调用服务器端开火函数
		ServerFire();
	}
}

// 服务器端开火处理，用于同步所有客户端的开火动作
void UCombatComponent::ServerFire_Implementation() const
{
	// 调用多播开火函数，实现所有客户端的同时开火效果
	MulticastFire();
}

// 多播开火实现，用于实际播放角色开火动画和执行武器开火逻辑
void UCombatComponent::MulticastFire_Implementation() const
{
	// 如果没有装备武器，则不执行任何操作
	if (EquippedWeapon == nullptr) return;

	// 如果角色存在
	if (Character)
	{
		// 播放角色开火动画，参数bAiming表示是否瞄准状态
		Character->PlayFireMontage(bAiming);

		// 调用装备武器的开火函数，实现实际开火逻辑
		EquippedWeapon->Fire(HitTargetLocation);
	}
}

// 在玩家的十字准星下进行线迹追踪，以检测是否击中了目标
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// 定义视口大小变量
	FVector2d ViewportSize;
	// 检查引擎和游戏视口是否有效
	if (GEngine && GEngine->GameViewport)
	{
		// 获取游戏视口大小
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 计算并获取游戏视口的中心位置
	const FVector2d CrosshairLocation(FVector2d(ViewportSize.X / 2.f, ViewportSize.Y / 2.f));

	// 定义世界坐标变量，用于存储视口中心位置的世界坐标和方向
	FVector CrosshairWorldPosition, CrosshairWorldDirection;

	// 将屏幕坐标转换为世界坐标
	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	// 如果转换成功
	if (bScreenToWorld)
	{
		// 定义射线起点
		const FVector Start = CrosshairWorldPosition;
		// 定义射线终点，通过方向和长度计算
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_LENGTH;

		// 执行单次线迹追踪
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// 如果没有命中任何物体
		if (!TraceHitResult.bBlockingHit)
		{
			// 将射线终点设置为撞击点
			TraceHitResult.ImpactPoint = End;
			HitTargetLocation = End;
		}
		else
		{
			HitTargetLocation = TraceHitResult.ImpactPoint;

			// 在命中位置绘制一个红色的调试球体
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red);
		}
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
