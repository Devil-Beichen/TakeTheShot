// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

/**
 *  弹壳
 */
UCLASS()
class TAKETHESHOT_API ACasing : public AActor
{
	GENERATED_BODY()

public:
	ACasing();

private:
	// 弹壳模型
	UPROPERTY(VisibleAnywhere, Category="Mesh")
	UStaticMeshComponent* CasingMesh;

	// 弹壳的射出力
	UPROPERTY(EditDefaultsOnly, Category="Shell")
	float ShellEjectionImpulse = 5.f;

	// 弹壳落地的音效
	UPROPERTY(EditDefaultsOnly, Category="Shell")
	class USoundCue* ShellSound;

protected:
	virtual void BeginPlay() override;

	/**
	 * 当此Actor与其他Actor发生碰撞时调用的函数。
	 * 
	 * @param HitComp  与此碰撞事件相关的组件。
	 * @param OtherActor  参与此碰撞事件的另一个Actor。
	 * @param OtherComp  参与此碰撞事件的另一个Actor的组件。
	 * @param NormalImpulse  碰撞产生的法向冲量，表示碰撞的力量和方向。
	 * @param Hit  碰撞的详细信息，包括碰撞点的位置和法线方向。
	 */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
