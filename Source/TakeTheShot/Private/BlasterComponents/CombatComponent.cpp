#include "BlasterComponents/CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlasterPlayerController.h"
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

	if (Character)
	{
		DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 如果Character对象存在，并且当前玩家控制该角色
	if (Character && Character->IsLocallyControlled())
	{
		// 设置HUD十字准星
		SetHUDCrosshairs(DeltaTime);

		// 初始化命中结果变量
		FHitResult HitResult;
		// 在十字准星位置进行射线检测，寻找命中对象
		TraceUnderCrosshairs(HitResult);
		// 记录命中点
		HitTarget = HitResult.ImpactPoint;

		// 平滑过渡视野（FOV），以增强游戏体验
		InterpFOV(DeltaTime);
	}
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
		// 设置武器状态为已装备
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		// 尝试找到角色右手的插槽，如果找到，则将武器装备到该插槽
		if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		
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
		Fire();
	}
}

// 开火
void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		bCanFire = false;
		// 调用服务器端开火函数
		ServerFire(HitTarget);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 1.75f;
		}

		StartFireTimer();
	}
}

// 开火定时器开始
void UCombatComponent::StartFireTimer()
{
	if (Character == nullptr || EquippedWeapon == nullptr)return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay,
		false
	);
}

// 开火定时器完成
void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;

	if (EquippedWeapon == nullptr)return;

	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

// 服务器端开火处理，用于同步所有客户端的开火动作
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget) const
{
	// 调用多播开火函数，实现所有客户端的同时开火效果
	MulticastFire(TraceHitTarget);
}

// 多播开火实现，用于实际播放角色开火动画和执行武器开火逻辑
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget) const
{
	// 如果没有装备武器，则不执行任何操作
	if (EquippedWeapon == nullptr) return;

	// 如果角色存在
	if (Character)
	{
		// 播放角色开火动画，参数bAiming表示是否瞄准状态
		Character->PlayFireMontage(bAiming);

		// 调用装备武器的开火函数，实现实际开火逻辑
		EquippedWeapon->Fire(TraceHitTarget);
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
		FVector Start = CrosshairWorldPosition;
		// 检查Character对象是否存在
		if (Character)
		{
			// 计算光标世界方向与Character位置之间的距离，并加上一个偏移量
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			// 根据计算出的距离，调整起始位置，以确保光标与Character之间的距离符合预期
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		// 定义射线终点，通过方向和长度计算
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_LENGTH;

		// 执行单次线迹追踪
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}

		// 如果没有命中任何物体
		if (!TraceHitResult.bBlockingHit)
		{
			// 将射线终点设置为撞击点
			TraceHitResult.ImpactPoint = End;
		}
		else
		{
			// 在命中位置绘制一个红色的调试球体
			/*DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red);*/
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(const float DeltaTime)
{
	// 检查Character和其控制器是否为空，若为空则不执行后续操作
	if (Character == nullptr || Character->Controller == nullptr) return;

	// 如果Controller为空，从Character的Controller中尝试获取一个ABlasterPlayerController类型的对象，否则使用现有的Controller
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		// 如果HUD为空，从Controller中尝试获取一个ABlasterHUD类型的对象，否则使用现有的HUD
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			// 如果装备了武器，将武器的十字准星信息复制到HUDPackage中
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				// 如果没有装备武器，将十字准星信息设置为nullptr
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// 计算十字准星的扩散

			// [0,600] ->[0,1] 将Character的速度映射到[0,1]的范围
			FVector2D WalkSpeedRange(0, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			// 根据速度计算十字准星的扩散因子
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			// 如果Character在空中，调整十字准星的扩散因子
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				// 如果Character不在空中，调整十字准星的扩散因子
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// 根据是否瞄准状态，调整瞄准因子
			if (bAiming)
			{
				// 当在瞄准状态时，将瞄准因子插值到目标值0.58
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				// 当不在瞄准状态时，将瞄准因子插值到目标值0
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// 无论状态如何，逐渐减少射击时的十字准星因子
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			// 设置HUDPackage的十字准星扩散值
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			// 将HUDPackage设置到HUD中
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

// 用于处理视场角（FOV）的插值变化
void UCombatComponent::InterpFOV(float DeltaTime)
{
	// 检查是否已经装备了武器，如果没有装备则直接返回
	if (EquippedWeapon == nullptr) return;

	// 根据是否处于瞄准状态，来决定插值的目标FOV和插值速度
	if (bAiming)
	{
		// 当处于瞄准状态时，FOV插值到装备武器的缩放FOV
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		// 不处于瞄准状态时，FOV插值回到默认FOV
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	// 如果Character存在且有跟随相机，则更新相机的FOV
	if (Character && Character->GetFollowCamera())
	{
		// 设置相机的FOV为当前计算得到的FOV
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
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
