// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/Team.h"
#include "GameFramework/Actor.h"
#include "FlagZone.generated.h"

/**
 * 夺旗得分区域
 */
UCLASS()
class TAKETHESHOT_API AFlagZone : public AActor
{
	GENERATED_BODY()

public:
	AFlagZone();

	// 团队
	UPROPERTY(EditAnywhere)
	ETeam Team = ETeam::ET_NoTeam;

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

	UPROPERTY()
	class ACaptureTheFlagGameMode* GameMode;

private:
	// 碰撞球体组件
	UPROPERTY(VisibleDefaultsOnly)
	class USphereComponent* ZoneSphere;

public:
};
