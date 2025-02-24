// Fill out your copyright notice in the Description page of Project Settings.


#include "Character//BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TakeTheShot.h"
#include "BlasterComponents/CombatComponent.h"
#include "BlasterComponents/BuffComponent.h"
#include "BlasterComponents/LagCompensationComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameMode/BlasterGameMode.h"
#include "GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerController/BlasterPlayerController.h"
#include "PlayerStart/TeamPlayerStart.h"
#include "PlayerState/BlasterPlayerState.h"
#include "Sound/SoundCue.h"
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
	// 设置尽量调整位置，但一定要生成
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 创建并初始化一个默认的Widget组件，用于显示头顶信息
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverlayWidget"));
	// 将新创建的Widget组件附加到根组件上
	OverheadWidget->SetupAttachment(RootComponent);

	// 设置角色的最大旋转速度
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 540.f);
	// 启用角色移动时的自动朝向功能
	GetCharacterMovement()->bOrientRotationToMovement = true;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));

	// 创建并初始化一个战斗组件，用于处理战斗相关逻辑
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	// 设置战斗组件为复制，使其在服务器和客户端之间同步
	Combat->SetIsReplicated(true);

	/**
	 * 创建并初始化一个 buff 组件，用于处理 buff 相关逻辑
	 */
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	/**
	 * 创建并初始化一个延迟补偿组件，用于处理延迟补偿逻辑
	 */
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	LagCompensation->SetIsReplicated(true);

	// 定义网络更新频率，用于控制数据同步的速率
	NetUpdateFrequency = 66.f;
	// 定义最小网络更新频率，确保在任何情况下更新频率都不会低于此值
	MinNetUpdateFrequency = 33.f;

	// 创建一个默认的静态网格体组件，用于表示角色携带的 Grenade
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	// 将新创建的静态网格体组件附加到角色的网格体上，
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	// 服务器回退碰撞检测盒（延迟补偿用）

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
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

	if (HasAuthority())
	{
		SpawnDefaultWeapon(); // 生成默认武器

		// 当角色受到任何伤害时，调用ReceiveDamage函数处理伤害逻辑
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	Initialize();
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

// 初始化
void ABlasterCharacter::Initialize()
{
	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	AddDefaultMappingContext(); // 添加默认的输入映射上下文

	UpdateHUDHealth(); // 更新 HUD 的健康值

	UpdateHUDShield(); // 更新 HUD 的护盾值

	UpdateHUDAmmo(); // 更新 HUD 的弹药数量
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	HideCameraIfCharacterClose();

	PollInit();
}

// 初始化角色的一些属性
void ABlasterCharacter::PollInit()
{
	// 检查BlasterPlayerState是否已经创建
	if (BlasterPlayerState == nullptr)
	{
		// 如果尚未创建，则尝试创建它
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

		// 如果BlasterPlayerState成功创建
		if (BlasterPlayerState)
		{
			OnPlayerStateInitialized(); // 玩家状态初始化
		}
	}
}

// 玩家状态初始化
void ABlasterCharacter::OnPlayerStateInitialized()
{
	// 初始化分数为0
	BlasterPlayerState->AddToScore(0.f);
	// 初始化死亡次数
	BlasterPlayerState->AddTotDefeats(0);
	// 设置队伍颜色
	SetTeamColor(BlasterPlayerState->GetTeam());
	SetSpawnPoint();

	// 如果游戏状态里面最高得分玩家有自己，就多播领先
	if (const ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
		{
			MulticastGainedTheLead();
		}
	}
}

// 为BlasterCharacter设置重生点
// 此函数仅在具有权威的情况下执行，确保玩家属于某个团队
void ABlasterCharacter::SetSpawnPoint()
{
	// 检查当前对象是否具有权威，并确保玩家不属于无团队
	if (HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		// 获取所有AActor类型的Actor，这些Actor的类与ATeamPlayerStart匹配
		TArray<AActor*> PlayerStats;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStats);

		// 存储与当前玩家团队匹配的ATeamPlayerStart实例
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto& Start : PlayerStats)
		{
			// 将找到的Actor转换为ATeamPlayerStart类型，并检查其团队是否与玩家的团队相同
			if (ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start); TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				// 如果团队匹配，则添加到TeamPlayerStarts数组中，避免重复添加
				TeamPlayerStarts.AddUnique(TeamStart);
			}
		}

		// 如果找到了合适的重生点，则随机选择一个重生点，并设置角色的位置和旋转
		if (!TeamPlayerStarts.IsEmpty())
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
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
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Started, this, &ABlasterCharacter::Aiming_Started);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &ABlasterCharacter::Aiming_Completed);

		// 绑定射击动作的触发事件
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlasterCharacter::Fire_Started);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlasterCharacter::Fire_Completed);

		// 绑定重装动作的开始事件
		EnhancedInputComponent->BindAction(Reload, ETriggerEvent::Started, this, &ABlasterCharacter::ReloadButtonPressed);

		// 绑定投掷手雷动作的开始事件
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &ABlasterCharacter::ThrowGrenadeStarted);
	}
}

// 初始化角色组件
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		// 设置初始速度
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched);
		// 设置初始跳跃速度
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
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

/**
* 只在服务器上执行的淘汰
* @param bPlayerLeftGame	是否玩家退出游戏 
*/
void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	// 丢弃或销毁武器
	DropOrDestrouWeapons();
	// 多播淘汰
	MulticastElim(bPlayerLeftGame);

	// 多播失去领先
	MulticastLostTheLead();
}

/**
* 多播淘汰
* @param bPlayerLeftGame	是否玩家退出游戏 
*/
void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
		BlasterPlayerController->SetHUDCarriedAmmo(0);
	}

	// 标记角色状态为已消除
	bEliminate = true;

	// 播放消除动画
	PlayElimMontage();

	// 移除输入映射上下文，停止响应用户输入
	// RemoveMappingContext();
	bDisableGameplay = true;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// 开始溶解效果
	StartDissolve();

	// 禁用移动模式，防止角色在消除后继续移动
	GetCharacterMovement()->DisableMovement();

	// 立即停止所有移动，确保角色固定不动
	GetCharacterMovement()->StopMovementImmediately();

	// 关闭碰撞，使角色不会与其他物体发生碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 关闭网格模型的碰撞，进一步确保没有碰撞检测
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 在位置生成一个消除机器人的效果
	SpawnElimBot();

	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	// 关闭武器瞄准
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	// 设置一个定时器，在指定延迟后调用ElimTimeFinished函数
	GetWorldTimerManager().SetTimer
	(
		ElimTime,
		this,
		&ABlasterCharacter::ElimTimeFinished,
		ElimDelayRegenerate
	);
}

// 当消除时间结束时调用
void ABlasterCharacter::ElimTimeFinished()
{
	// 如果游戏模式存在且玩家没有退出游戏，则请求重生角色
	if (BlasterGameMode && !bLeftGame)
	{
		// 请求重生，传递当前角色和控制器的信息
		BlasterGameMode->RequestRespawn(this, Controller);
		return;
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

// 多播获得领先
// 当玩家获得领先时调用此函数，以视觉效果指示领先状态
void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	// 检查冠冕系统是否已初始化
	if (CrownSystem == nullptr) return;

	// 如果冠冕组件尚未创建，则进行创建
	if (CrownComponent == nullptr)
	{
		// 附着冠冕系统到角色的胶囊组件上，位置略高于角色头部
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached
		(
			CrownSystem,
			GetMesh(),
			FName(),
			GetMesh()->GetComponentLocation() + FVector(0.f, 0.f, 220.f),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition,
			false
		);
	}

	// 如果冠冕组件已成功创建，则激活它
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}


// 多播失去领先
void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

// 离开游戏
void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	if (bLeftGame)return;
	// 尝试获取游戏模式并进行类型检查，确保它是ABlasterGameMode的实例
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (BlasterGameMode)
	{
		BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
		if (BlasterPlayerState)
		{
			// 请求离开游戏
			BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
		}
	}
}


// 生成淘汰机器人的效果和声音
void ABlasterCharacter::SpawnElimBot()
{
	// 如果淘汰机器人效果存在，则生成效果
	if (ElimBotEffect)
	{
		// 计算淘汰机器人生成的位置，Z轴提高200.f单位以避免地面遮挡
		const FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);

		// 在计算出的位置生成淘汰机器人效果
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}

	// 如果淘汰机器人声音存在，则生成声音
	if (ElimBotSound)
	{
		// 在当前角色位置生成淘汰机器人声音
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
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

// 换弹动作
void ABlasterCharacter::PlayReloadMontage_Implementation()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			FName SectionName;
			switch (Combat->EquippedWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("RocketLauncher");
				break;
			case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;
			case EWeaponType::EWT_SubmachineGun:
				SectionName = FName("Pistol");
				break;
			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Shotgun");
				break;
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("SniperRifle");
				break;
			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("GrenadeLauncher");
				break;
			default:
				break;
			}
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}

// 换弹结束
void ABlasterCharacter::FinishReload()
{
	if (Combat == nullptr) return;

	Combat->FinishReload();
}

void ABlasterCharacter::Shell()
{
	if (Combat == nullptr) return;

	Combat->ShotgunShellReload();
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr || Combat->CombatState != ECombatState::ECS_Unoccupied)return;

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

// 淘汰动画
void ABlasterCharacter::PlayElimMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (ElimMontage)
		{
			AnimInstance->Montage_Play(ElimMontage);
		}
	}
}

// 抛掷 手榴弹 动画
void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (ThrowGrenadeMontage)
		{
			AnimInstance->Montage_Play(ThrowGrenadeMontage);
		}
	}
}

// 切换武器动画
void ABlasterCharacter::PlaySwapMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (SwapMontage)
		{
			AnimInstance->Montage_Play(SwapMontage);
		}
	}
}

// 发射手雷
void ABlasterCharacter::LaunchGrenade()
{
	if (Combat)
	{
		Combat->LaunchGrenade();
	}
}

// 抛掷手榴弹结束
void ABlasterCharacter::ThrowGrenadeFinished()
{
	if (Combat)
	{
		Combat->ThrowGrenadeFinished();
	}
}

// 切换武器绑定
void ABlasterCharacter::SwapAttachWeapon()
{
	if (Combat)
	{
		Combat->SwapAttachWeapon();
	}
}

// 切换完成
void ABlasterCharacter::SwapFinish()
{
	if (Combat)
	{
		Combat->SwapFinish();
	}
}

void ABlasterCharacter::Destroyed()
{
	// 如果ElimBotComponent存在
	if (ElimBotComponent)
	{
		// 销毁ElimBotComponent组件
		ElimBotComponent->DestroyComponent();
	}

	// 判断比赛状态是否不在进行中
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	// 如果Combat组件存在、装备的武器存在且比赛不在进行中，则销毁装备的武器
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}

	Super::Destroyed();
}

// 当角色受到伤害时调用此函数
// 参数:
// - DamagedActor: 受伤的角色对象
// - Damage: 伤害量
// - DamageType: 指向损伤类型的指针
// - InstigatorController: 施加伤害的控制器对象
// - DamageCauser: 造成伤害的角色对象
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	// 如果已经死亡就返回
	if (bEliminate || BlasterGameMode == nullptr) return;

	// 添加日志记录，记录每次调用的详细信息
	UE_LOG(LogTemp, Log, TEXT("%s 受到了 %f点伤害，调用者: %s, 损伤来源: %s"),
	       *this->GetName(), Damage,
	       InstigatorController ? *InstigatorController->GetName() : TEXT("None"),
	       DamageCauser ? *DamageCauser->GetName() : TEXT("None"));
	Damage = BlasterGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	// 实际收到的伤害
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			// 伤害减去护盾
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			// 护盾归零
			Shield = 0.f;
		}
	}

	// 更新生命值，并确保它不会超过最大值或低于0
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	// 更新HUD上的生命值显示
	UpdateHUDHealth();
	// 更新HUD上的护盾值显示
	UpdateHUDShield();

	// 如果生命值为0，则角色死亡
	if (Health == 0.f)
	{
		// 获取当前游戏模式并检查是否为ABlasterGameMode类型
		if (BlasterGameMode)
		{
			// 尝试获取并转换BlasterPlayerController对象
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			// 获取并转换施加伤害的控制器为ABlasterPlayerController类型
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			// 通知游戏模式角色已被淘汰
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
	else
	{
		// 确实受到了伤害
		if (Damage != 0.f)
		{
			// 如果角色没有死亡，则播放受伤反应动画
			PlayHitReactMontage();
		}
	}
}

// 移动函数，根据输入值调整角色的移动方向
void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

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
	if (bDisableGameplay) return;
	if (Combat->bHoldingTheFlag) return;
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
	if (bDisableGameplay) return;
	if (Combat->bHoldingTheFlag) return;
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
	ServerSlowStarted();
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
		if (Buff)
		{
			// 如果是在加速的时候获得的
			WalkSpeed = Buff->bAccelerationbuff ? GetCharacterMovement()->MaxWalkSpeed : Buff->InitialBaseSpeed;
		}

		GetCharacterMovement()->MaxWalkSpeed = SlowWalkSpeed;
	}
	else
	{
		bIsSlowWalk = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

// 拾取装备按钮开始函数，当按下装备按钮时被调用
void ABlasterCharacter::Equip_Started()
{
	if (bDisableGameplay) return;
	if (!Combat) return;
	if (Combat->bHoldingTheFlag) return;
	if (Combat->CombatState == ECombatState::ECS_Unoccupied)ServerEquipButtonPressed();
	bool bSwap = Combat->ShouldSwapWeapons() &&
		IsLocallyControlled() &&
		!HasAuthority() &&
		Combat->CombatState == ECombatState::ECS_Unoccupied &&
		OverlappingWeapon == nullptr;
	if (bSwap)
	{
		PlaySwapMontage();
		Combat->CombatState = ECombatState::ECS_SwappingWeapons;
		bFinishedSwapping = false;
	}
}

// 服务器调用，确保所有客户端都能正确地接收到装备按钮按下的消息并做出相应调整。
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	// 如果战斗组件有效
	if (Combat)
	{
		// 如果有重叠的武器，则装备该武器
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		// 如果重叠武器无效并且用于副武器，就进行武器交互
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

// 当角色启用瞄准时调用此函数
void ABlasterCharacter::Aiming_Started()
{
	if (bDisableGameplay) return;
	if (!Combat) return;
	if (Combat->bHoldingTheFlag) return;
	// 检查是否存在Combat组件，是否装备了武器，以及角色是否未在下落状态
	if (Combat->EquippedWeapon && !GetCharacterMovement()->IsFalling())
	{
		// 设置瞄准状态为true，启用瞄准
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::Aiming_Completed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::Fire_Started()
{
	if (bDisableGameplay) return;
	if (!Combat)return;
	if (Combat->bHoldingTheFlag) return;

	Combat->FireButtonPressed(true);
}

void ABlasterCharacter::Fire_Completed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

// 重新装填按下
void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->bHoldingTheFlag) return;
		Combat->Reload();
	}
}

// 投掷 Grenade 按钮按下
void ABlasterCharacter::ThrowGrenadeStarted()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		if (Combat->bHoldingTheFlag) return;
		Combat->ThrowGrenade();
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
		if (Combat)
		{
			// 如果存在装备的武器，则设置其网格模型为不可见。
			if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			{
				Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
			}
			// 同理，如果存在副武器，也设置其网格模型为不可见。
			if (Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
			{
				Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
			}
		}
	}
	else
	{
		// 如果距离大于等于阈值，恢复角色模型的可见性。
		GetMesh()->SetVisibility(true);
		if (Combat)
		{
			// 如果存在装备的武器，则设置其网格模型为可见。
			if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			{
				Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
			}
			// 同理，如果存在副武器，也设置其网格模型为可见。
			if (Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
			{
				Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
			}
		}
	}
}

// 设置重叠武器
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

// 设置重叠武器回调
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) const
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

// 判断当前角色是否装备了武器
bool ABlasterCharacter::IsWeaponEquipped() const
{
	return (Combat && Combat->EquippedWeapon);
}

// 判断当前角色是否瞄准
bool ABlasterCharacter::IsAiming() const
{
	return (Combat && Combat->bAiming);
}

// 更新HUD上的生命值
void ABlasterCharacter::UpdateHUDHealth()
{
	// 将控制器转换为BlasterPlayerController类型，并更新生命值
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

// 更新HUD上的护盾值
void ABlasterCharacter::UpdateHUDShield()
{
	// 将控制器转换为BlasterPlayerController类型，并更新生命值
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

// 更新HUD上的弹药信息
void ABlasterCharacter::UpdateHUDAmmo()
{
	// 将控制器转换为BlasterPlayerController类型，并更新生命值
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	// 如果存在控制器和战斗组件，则更新HUD上的弹药信息
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

// 创建默认武器
void ABlasterCharacter::SpawnDefaultWeapon()
{
	// 如果默认武器类存在，并且当前对象有权限，并且不是淘汰状态
	if (DefaultWeaponClass && HasAuthority() && !bEliminate)
	{
		// 获取当前游戏模式并转换为ABlasterGameMode类型
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		// 获取当前世界对象
		UWorld* World = GetWorld();
		// 如果游戏模式和世界对象都有效
		if (BlasterGameMode && World)
		{
			// 创建默认武器
			AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
			// 设置武器的销毁标志为true，表示这把武器在一定条件下需要被销毁
			StartingWeapon->bDestroyWeapon = true;
			// 如果当前对象的战斗组件存在
			if (Combat)
			{
				// 让战斗组件装备新创建的武器
				Combat->EquipWeapon(StartingWeapon);
			}
		}
	}
}

// 删除默认生成的武器
void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;

	// 如果武器的销毁标志为true，则直接销毁武器，否则将其丢弃
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

// 删除或者丢弃丢弃所有武器
void ABlasterCharacter::DropOrDestrouWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
	}
}

// 在原地旋转角色
// 
// 此函数负责在游戏过程中处理角色的原地旋转逻辑。根据角色的网络角色和是否被禁用，
// 它会执行不同的操作来更新角色的瞄准偏移或处理运动的复制。
//
// 参数:
// - DeltaTime: 自上次调用以来的时间（以秒为单位）
void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		// 禁用控制器旋转
		bUseControllerRotationYaw = false;
		// 禁用角色的移动方向与旋转方向的自动对齐
		GetCharacterMovement()->bOrientRotationToMovement = true;
		// 设置为未转动状态
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		// 返回，不再执行后续代码
		return;
	}

	// 如果游戏玩法被禁用，则直接返回，不执行任何操作
	if (bDisableGameplay)
	{
		// 禁用控制器旋转
		bUseControllerRotationYaw = false;
		// 设置为未转动状态
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		// 返回，不再执行后续代码
		return;
	}

	// 如果当前角色的本地角色不是模拟代理（SimulatedProxy）或者是由本地玩家控制的，
	// 则根据Delta_time更新瞄准偏移
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		// 如果不是本地控制的角色，则增加自上次运动复制以来的时间
		TimeSinceLastMovementReplication += DeltaTime;
		// 如果自上次运动复制以来的时间超过0.25秒，则调用函数处理运动的复制
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

		// 计算和更新瞄准偏移的俯仰角
		CalculateAO_Pitch();
	}
}

// 当角色的生命值发生变化时调用此函数，用于处理相关的逻辑
void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

// 当角色的护盾值发生变化时调用此函数，用于处理相关的逻辑
void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

// 设置队伍颜色
void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		if (OriginalMaterial)
		{
			GetMesh()->SetMaterial(0, OriginalMaterial);
		}
		break;
	case ETeam::ET_RedTeam:
		if (RedMaterial)
		{
			GetMesh()->SetMaterial(0, RedMaterial);
		}
		break;
	case ETeam::ET_BlueTeam:
		if (BlueMaterial)
		{
			GetMesh()->SetMaterial(0, BlueMaterial);
		}
		break;
	case ETeam::ET_MAX:
		break;
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(const float DissolveValue)
{
	GetMesh()->SetScalarParameterValueOnMaterials("Dissolve", DissolveValue);
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->SetPlayRate(1.f / DissolveTime);
		DissolveTimeline->Play();
	}
}

// 获取当前装备的武器
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

// 获取当前角色的队伍
ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr)return ETeam::ET_NoTeam;
	return BlasterPlayerState->GetTeam();
}

// 设置当前角色是否持有旗帜
void ABlasterCharacter::SetbHoldingTheFlag(bool bHolding)
{
	if (Combat)
	{
		Combat->bHoldingTheFlag = bHolding;
	}
}
