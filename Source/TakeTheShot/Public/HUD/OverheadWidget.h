// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 *	在头顶显示的窗口小部件
 */
UCLASS()
class TAKETHESHOT_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	属性DisplayText用于在UI中显示文本内容
	使用BindWidget元数据以便在蓝图中将其绑定到UI控件
	*/
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	/**
	 * 设置显示文本。
	 * 
	 * 此函数用于设置要显示的文本。它接受一个指向FString的常量指针，该指针指向要显示的文本。
	 * 使用常量指针确保了函数不会修改传入的文本，保证了文本的原始状态。
	 * 
	 * @param TextToDisplay 指向要显示的文本的常量指针。
	 */
	void SetDisplayText(const FString* const TextToDisplay) const;

	/**
	 * 显示玩家的网络角色。
	 * 
	 * 此函数用于显示指定玩家的网络角色信息。它接受一个指向APawn的常量指针，该指针指向要显示信息的玩家角色。
	 * 使用常量指针确保了函数不会修改角色对象，保证了角色状态的完整性。
	 * 
	 * @param InPawn 指向要显示网络角色信息的玩家角色的常量指针。
	 */
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(const APawn* const InPawn) const;

protected:
	virtual void NativeDestruct() override;
};
