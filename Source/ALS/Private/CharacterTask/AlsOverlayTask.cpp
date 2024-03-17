// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterTasks/AlsOverlayTask.h"
#include "AlsCharacter.h"
#include "LinkedAnimLayers/AlsOverlayAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverlayTask)

void UAlsOverlayTask::Begin()
{
	Super::Begin();

	if (IsValid(OverlayAnimInstance))
	{
		Character->GetMesh()->LinkAnimClassLayers(OverlayAnimInstance);
	}
}

void UAlsOverlayTask::OnEnd(bool bWasCancelled)
{
	Super::OnEnd(bWasCancelled);

	if (Character.IsValid() && IsValid(OverlayAnimInstance))
	{
		Character->GetMesh()->UnlinkAnimClassLayers(OverlayAnimInstance);
	}
}
