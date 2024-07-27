// Fill out your copyright notice in the Description page of Project Settings.


#include "Character//BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
