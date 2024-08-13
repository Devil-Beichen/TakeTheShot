// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 检查是否存在炮弹音效资源
	if (ShellSound)
	{
		// 如果存在，使用UGameplayStatics::SpawnSoundAtLocation函数在当前Actor的位置播放音效
		UGameplayStatics::SpawnSoundAtLocation(this, ShellSound, GetActorLocation());
		ShellSound = nullptr;
	}
	
	// 定义一个计时器句柄，用于在未来的某个时间点触发事件
	FTimerHandle DestroyTimer;

	// 设置一个一次性计时器，用于在0.5秒后调用ACasing类的K2_DestroyActor方法
	// 此处使用SetTimer方法，其参数分别为计时器句柄、对象指针、成员函数指针、触发时间间隔、是否重复
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ACasing::K2_DestroyActor, 0.5f, false);
}
