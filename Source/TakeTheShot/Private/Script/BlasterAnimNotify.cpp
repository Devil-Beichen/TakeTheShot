// Fill out your copyright notice in the Description page of Project Settings.


#include "Script/BlasterAnimNotify.h"

FString UBlasterAnimNotify::GetNotifyName_Implementation() const
{
	return FunctionName.ToString();
}

void UBlasterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp) return;
	if (AActor* Actor = MeshComp->GetOwner())
	{
		if (UFunction* TempFunction = Actor->FindFunction(FunctionName))
		{
			Actor->ProcessEvent(TempFunction, nullptr);
		}
	}
}
