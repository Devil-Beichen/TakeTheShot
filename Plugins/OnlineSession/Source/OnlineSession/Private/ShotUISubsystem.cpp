// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotUISubsystem.h"

// 匹配类型
EMatchType UShotUISubsystem::DesiredMatchType{};

// 创建子系统
bool UShotUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

// 初始化子系统
void UShotUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

// 销毁子系统
void UShotUISubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// 设置匹配类型
void UShotUISubsystem::SetDesiredMatchType(const UObject* WorldContextObject, EMatchType MatchType)
{
	DesiredMatchType = MatchType;
}
