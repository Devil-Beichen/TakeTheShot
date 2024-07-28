// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**	大厅游戏模式基类
 * 
 */
UCLASS()
class TAKETHESHOT_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

	ALobbyGameMode();
	
public:
	/**	在玩家登录后调用的函数。
	* 该函数被设计为在玩家控制器成功登录后被调用。它提供了一个钩子，
	* 允许子类在登录过程完成后执行自定义逻辑。例如，可以使用这个函数来初始化
	* 玩家的状态，加载玩家的存档，或者设置玩家的初始位置和朝向。
	* @param NewPlayer 指向新登录玩家控制器的指针。这个参数允许函数与特定的玩家交互，比如设置玩家的视角或者向玩家发送消息。
	*/
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 在BP中实现玩家登录
	UFUNCTION(BlueprintImplementableEvent)
	void PostLogin_BP(APlayerController* NewPlayer);
};
