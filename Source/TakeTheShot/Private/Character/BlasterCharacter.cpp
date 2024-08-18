// Fill out your copyright notice in the Description page of Project Settings.


#include "Character//BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TakeTheShot.h"
#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

ABlasterCharacter::ABlasterCharacter()
{
	// 设置本Actor不会参与Tick更新，优化性能
	PrimaryActorTick.bCanEverTick = true;

	// 创建一个默认的SpringArm组件，用于持有和管理摄像机的位置
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// 将CameraBoom组件附加到Actor的主体上
	CameraBoom->SetupAttachment(GetMesh());
	// 将摄像机Boom的位置设置为世界坐标系中的(0, 0, 160)
	CameraBoom->SetWorldLocation(FVector(0.f, 0.f, 160.f));
	// 设置CameraBoom的目标臂展长度，即摄像机与角色之间的理想距离
	CameraBoom->TargetArmLength = 600.f;
	// 允许CameraBoom根据角色的朝向自动调整摄像机的朝向
	CameraBoom->bUsePawnControlRotation = true;

	// 创建一个默认的摄像机组件，用于实际的摄像机视图
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	// 将摄像机组件附加到CameraBoom上，实现摄像机的跟随效果
	FollowCamera->SetupAttachment(CameraBoom);
	// 关闭跟随摄像机的棋子控制旋转功能
	FollowCamera->bUsePawnControlRotation = false;

	// 允许角色蹲伏
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	// 关闭与相机的碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	// 设置网格体碰撞的通道类型为自定义的网格体碰撞
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 创建并初始化一个默认的Widget组件，用于显示头顶信息
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverlayWidget"));
	// 将新创建的Widget组件附加到根组件上
	OverheadWidget->SetupAttachment(RootComponent);

	// 设置角色的最大旋转速度
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 540.f);
	// 启用角色移动时的自动朝向功能
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 创建并初始化一个战斗组件，用于处理战斗相关逻辑
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	// 设置战斗组件为复制，使其在服务器和客户端之间同步
	Combat->SetIsReplicated(true);

	// 定义网络更新频率，用于控制数据同步的速率
	NetUpdateFrequency = 66.f;
	// 定义最小网络更新频率，确保在任何情况下更新频率都不会低于此值
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;
	AddDefaultMappingContext();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

		CalculateAO_Pitch();
	}

	HideCameraIfCharacterClose();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定跳跃动作的开始事件 绑定跳跃动作的触发事件和完成事件
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABlasterCharacter::Jump_Started);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABlasterCharacter::Jump_Completed);

		// 绑定蹲下动作的开始事件
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::Crouch_Started);
		// 绑定装备动作的开始事件
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlasterCharacter::Equip_Started);
		// 绑定慢速动作的开始事件
		EnhancedInputComponent->BindAction(SlowAction, ETriggerEvent::Started, this, &ABlasterCharacter::Slow_Started);
		// 绑定移动动作的触发事件
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		// 绑定查看动作的触发事件
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);

		/*绑定瞄准动作的触发事件和完成事件*/
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Aiming_Triggered);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &ABlasterCharacter::Aiming_Completed);

		// 绑定射击动作的触发事件
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlasterCharacter::Fire_Started);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlasterCharacter::Fire_Completed);
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

#pragma region 绑定按键相关
void ABlasterCharacter::AddDefaultMappingContext()
{
	// 尝试将控制器转换为玩家控制器对象
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		// 尝试从本地玩家获取增强输入子系统
		EnhancedSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (EnhancedSubsystem)
		{
			// 为默认映射上下文添加一个映射上下文
			EnhancedSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

/**
 * 移除映射上下文。
 * 
 * 本函数用于从增强子系统中移除默认映射上下文。只有在增强子系统和默认映射上下文都存在的情况下才会执行移除操作。
 * 这可能是为了暂时或永久地中断特定于映射的处理逻辑，或者在系统重新配置时清理资源。
 * 
 * @note 本函数是常量成员函数，这意味着它承诺不会修改类的任何成员变量。
 */
void ABlasterCharacter::RemoveMappingContext() const
{
	// 检查增强子系统和默认映射上下文是否存在
	if (EnhancedSubsystem && DefaultMappingContext)
	{
		// 从增强子系统中移除默认映射上下文
		EnhancedSubsystem->RemoveMappingContext((DefaultMappingContext));
	}
}

void ABlasterCharacter::PlayFireMontage(const bool bAiming) const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FireWeaponMontage)
		{
			AnimInstance->Montage_Play(FireWeaponMontage);
			const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}

void ABlasterCharacter::MulticastPlayHitReactMontage_Implementation()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (HitReactMontage)
		{
			AnimInstance->Montage_Play(HitReactMontage);
			const FName SectionName("FromFront");
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}

// 移动函数，根据输入值调整角色的移动方向
void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	// 检查控制器是否存在，如果不存在则直接返回
	if (!Controller) return;

	// 获取输入的移动向量
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// 构造一个旋转器，其Y轴旋转角度为角色的控制旋转角度
	const FRotator Rotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);

	// 计算角色的向前方向
	const FVector ForwardDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);

	// 计算角色的向右方向
	const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

	// 根据输入向量的Y分量，添加向前的移动输入
	AddMovementInput(ForwardDirection, MovementVector.X);
	// 根据输入向量的X分量，添加向右的移动输入
	AddMovementInput(RightDirection, MovementVector.Y);
}

// 注视函数，根据输入值调整角色的注视方向
void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	// 获取输入的注视向量
	const FVector2D LookAxisValue = Value.Get<FVector2D>();

	// 如果控制器存在，则根据输入向量调整角色的注视方向
	if (Controller)
	{
		// 根据输入向量的X分量，添加Y轴的旋转输入
		AddControllerYawInput(LookAxisValue.X);
		// 根据输入向量的Y分量，添加X轴的旋转输入
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

// 跳跃开始函数，当跳跃动作开始时被调用
void ABlasterCharacter::Jump_Started()
{
	// 如果角色处于蹲伏状态，则取消蹲伏；否则执行跳跃动作
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Jump();
	}
}

// 跳跃完成函数，当跳跃动作完成时被调用
void ABlasterCharacter::Jump_Completed()
{
	// 停止跳跃动作
	StopJumping();
}

// 蹲伏开始函数，当蹲伏动作开始时被调用
void ABlasterCharacter::Crouch_Started()
{
	// 根据角色当前的状态，决定是取消蹲伏还是开始蹲伏
	// 如果角色处于蹲伏状态，则取消蹲伏；否则执行蹲伏动作
	if (bIsCrouched)
	{
		// 当角色处于蹲伏状态时，调用UnCrouch函数来取消蹲伏
		UnCrouch();
	}
	else if (GetCharacterMovement()->IsFalling() == false)
	{
		// 当角色不处于下落状态时，调用Crouch函数来开始蹲伏
		Crouch();
	}
}

/* 当角色开始减速时调用此函数
如果客户端没有控制权，则请求服务器开始减速
如果有控制权，则直接通知所有客户端开始减速*/
void ABlasterCharacter::Slow_Started()
{
	if (!HasAuthority())
	{
		ServerSlowStarted();
	}
	else
	{
		MulticastSlowStarted();
	}
}

// 该函数被服务器调用，以确保所有客户端都能正确地接收到减速状态并作出相应调整。
void ABlasterCharacter::ServerSlowStarted_Implementation()
{
	// 设置角色的速度，这是实现加速减速效果的关键步骤。
	MulticastSlowStarted();
}

// 多播设置速度的函数，用于在客户端上设置角色的速度。
void ABlasterCharacter::MulticastSlowStarted_Implementation()
{
	if (bIsCrouched || GetCharacterMovement()->IsFalling()) return;

	if (bIsSlowWalk == false)
	{
		bIsSlowWalk = true;
		GetCharacterMovement()->MaxWalkSpeed = SlowWalkSpeed;
	}
	else
	{
		bIsSlowWalk = false;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

// 拾取装备按钮开始函数，当按下装备按钮时被调用
void ABlasterCharacter::Equip_Started()
{
	if (!Combat) return;

	if (HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

// 当角色启用瞄准时调用此函数
void ABlasterCharacter::Aiming_Triggered()
{
	// 检查是否存在Combat组件，是否装备了武器，以及角色是否未在下落状态
	if (Combat && Combat->EquippedWeapon && !GetCharacterMovement()->IsFalling())
	{
		// 设置瞄准状态为true，启用瞄准
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::Aiming_Completed()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::Fire_Started()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::Fire_Completed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

#pragma endregion

/**
 * 如果角色靠近相机，隐藏相机以防止穿模。
 * 这个函数主要用于解决角色靠近相机时可能出现的穿透问题，通过判断角色与相机的距离，
 * 来决定是否隐藏角色和武器的网格模型。
 */
void ABlasterCharacter::HideCameraIfCharacterClose()
{
	// 如果当前角色不是本地控制的，则无需执行隐藏操作。
	if (!IsLocallyControlled()) return;

	// 获取相机位置与角色位置的距离。
	float Distance = (FollowCamera->GetComponentLocation() - GetActorLocation()).Size();
	// 如果距离小于设定的阈值，则隐藏角色模型。
	if (Distance < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		// 如果存在战斗状态，并且装备了武器，同时武器的网格模型存在，则设置武器模型为不可见。
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		// 如果距离大于等于阈值，恢复角色模型的可见性。
		GetMesh()->SetVisibility(true);
		// 同样地，如果存在战斗状态且装备了武器，则恢复武器模型的可见性。
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// 如果存在重叠的武器对象是本地控制的
	if (OverlappingWeapon && IsLocallyControlled())
	{
		// 隐藏重叠武器的拾取提示 widget
		OverlappingWeapon->ShowPickupWidget(false);
	}

	// 将当前武器对象赋值给重叠武器变量
	OverlappingWeapon = Weapon;

	// 如果当前对象是本地控制的并且存在重叠的武器对象
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		// 显示重叠武器的拾取提示 widget
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const
{
	// 如果上一个被触及的武器（LastWeapon）存在，则将其拾取提示设置为false，表示隐藏拾取提示
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}

	// 如果当前有武器重叠（OverlappingWeapon），则将重叠武器的拾取提示设置为true，表示显示拾取提示
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const
{
	return (Combat && Combat->bAiming);
}

void ABlasterCharacter::OnRep_Health()
{
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

void ABlasterCharacter::AimOffset(const float DeltaTime)
{
	// 如果战斗组件没有装备武器，则返回
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	const bool bIsInAir = GetCharacterMovement()->IsFalling();
	float Speed = CalculateSpeed();
	// 如果速度为0且角色不在空中，则执行以下操作
	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;

		// 禁用控制器yaw旋转，因为我们将使用输入直接控制yaw
		bUseControllerRotationYaw = true;

		// 当前的瞄准旋转
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// 计算当前瞄准旋转与初始瞄准旋转之间的差值
		const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = FMath::FInterpTo(AO_Yaw, DeltaRotation.Yaw, DeltaTime, 20.f);

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // 如果速度大于0或角色在空中，则执行以下操作
	{
		bRotateRootBone = false;
		// 启用控制器的yaw旋转，这通常用于第一人称视角角色，以使角色的朝向与控制器的输入相匹配。
		bUseControllerRotationYaw = true;

		// 获取角色的初始瞄准旋转
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	// 获取基础瞄准旋转的俯仰角
	AO_Pitch = GetBaseAimRotation().Pitch;
	// 如果俯仰角大于90度且当前对象不是本地控制的
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// 将Pitch角从[270,360]映射到[-90,0]
		const FVector2d InRange(270.f, 360.f);
		const FVector2d OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

// 模拟代理旋转行为
void ABlasterCharacter::SimProxiesTurn()
{
	// 如果战斗组件或装备的武器为空，则不执行旋转逻辑
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	// 计算当前速度
	float Speed = CalculateSpeed();

	// 初始化根骨骼旋转标志为false
	bRotateRootBone = false;

	// 如果速度大于0，则视为未在原地转动
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	// 保存上一帧的代理旋转
	ProxyRotationLastFrame = ProxyRotation;
	// 获取当前代理旋转
	ProxyRotation = GetActorRotation();
	// 计算当前帧与上一帧代理旋转的差异，并归一化
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	// 判断是否超过转动阈值
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		// 根据转动方向设置转动状态
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	// 如果未超过转动阈值，则视为未在原地转动
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}


/**
 * 在原地转动角色。
 * 该函数根据Delta Time参数来控制角色在原地的转动行为。
 * 它通过判断角色的偏航角（AO_Yaw）来确定角色是向左转、向右转还是停止转动。
 * 
 * @param DeltaTime 两次调用之间的时间间隔，用于平滑转动动画。
 */
void ABlasterCharacter::TurnInPlace(const float DeltaTime)
{
	// 判断是否向右转动
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	// 判断是否向左转动
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	// 如果角色处于转动状态（不是未转动状态）
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// 平滑地将InterpAO_Yaw插值到0，以实现转动效果
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		// 更新AO_Yaw以反映转动
		AO_Yaw = InterpAO_Yaw;
		// 当接近正向时，停止转动并重置起始瞄准旋转
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
