// Fill out your copyright notice in the Description page of Project Settings.


#include "Character//BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

ABlasterCharacter::ABlasterCharacter()
{
	// 设置本Actor不会参与Tick更新，优化性能
	PrimaryActorTick.bCanEverTick = false;

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

	// 创建并初始化一个默认的Widget组件，用于显示头顶信息
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverlayWidget"));
	// 将新创建的Widget组件附加到根组件上
	OverheadWidget->SetupAttachment(RootComponent);

	// 创建并初始化一个战斗组件，用于处理战斗相关逻辑
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	// 设置战斗组件为复制，使其在服务器和客户端之间同步
	Combat->SetIsReplicated(true);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	AddDefaultMappingContext();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定跳跃动作的开始事件
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABlasterCharacter::Jump_Started);
		// 绑定跳跃动作的完成事件
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABlasterCharacter::Jump_Completed);
		// 绑定蹲下动作的开始事件
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::Crouch_Started);
		// 绑定装备动作的开始事件
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlasterCharacter::Equip_Started);
		// 绑定移动动作的触发事件
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		// 绑定查看动作的触发事件
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
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

#pragma endregion

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
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
