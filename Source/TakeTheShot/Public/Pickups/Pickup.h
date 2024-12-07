// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

/**
 * 拾取类
 */
UCLASS()
class TAKETHESHOT_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();
	virtual void Tick(float DeltaTime) override;
	// 销毁时调用
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	/**
	 * 当角色的碰撞球体与另一个演员的碰撞体发生重叠时，此函数将被调用。
	* 
	* @param OverlappedComponent 角色的碰撞球体组件，表示触发重叠的组件。
	* @param OtherActor 发生重叠的另一个演员。
	* @param OtherComp 另一个演员的碰撞组件。
	* @param OtherBodyIndex 另一个碰撞体的索引。
	* @param bFromSweep 如果重叠是由于扫动检测引起的，则此参数为true；否则为false。
	* @param SweepResult 扫动检测的结果，包含重叠的详细信息。
	*/
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 旋转速度
	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 45.f;

private:
	// 拾取球体组件
	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* OverlapSphere;

	// 拾取音效
	UPROPERTY(EditDefaultsOnly)
	class USoundCue* PickupSound;

	// 拾取网格体
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* PickupMesh;

	// 显示特效
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponents;

	// 拾取特效
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

public:
};
