// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterTasks/AlsOverlayTask.h"
#include "AlsCharacter.h"
#include "LinkedAnimLayers/AlsOverlayAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverlayTask)

void UAlsOverlayTask::Begin()
{
	if (!bActive && IsValid(OverlayAnimClass))
	{
		OverlayAnimInstance = Cast<UAlsOverlayAnimInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverlayAnimClass));

		if (!OverlayAnimInstance.IsValid())
		{
			Character->GetMesh()->LinkAnimClassLayers(OverlayAnimClass);
			OverlayAnimInstance = Cast<UAlsOverlayAnimInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverlayAnimClass));
		}

		OverlayAnimInstance->Refresh(this);
	}
	Super::Begin();
}

void UAlsOverlayTask::Refresh(float DeltaTime)
{
	Super::Refresh(DeltaTime);
	if (OverlayAnimInstance.IsValid())
	{
		OverlayAnimInstance->Refresh(this);
	}
}

void UAlsOverlayTask::OnFinished()
{
	if (Character.IsValid() && IsValid(OverlayAnimClass))
	{
		Character->GetMesh()->UnlinkAnimClassLayers(OverlayAnimClass);
		OverlayAnimInstance.Reset();
	}

	Super::OnFinished();
}
