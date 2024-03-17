// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterTasks/AlsOverrideTask.h"
#include "AlsCharacter.h"
#include "AlsLinkedAnimationInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverrideTask)

void UAlsOverrideTask::Begin()
{
	Super::Begin();

	if (IsValid(OverrideAnimClass))
	{
		OverrideAnimInstance = Cast<UAlsLinkedAnimationInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverrideAnimClass));

		if (!OverrideAnimInstance.IsValid())
		{
			Character->GetMesh()->LinkAnimClassLayers(OverrideAnimClass);
			OverrideAnimInstance = Cast<UAlsLinkedAnimationInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverrideAnimClass));
		}
	}
}

void UAlsOverrideTask::OnEnd(bool bWasCancelled)
{
	Super::OnEnd(bWasCancelled);

	if (Character.IsValid() && IsValid(OverrideAnimClass))
	{
		Character->GetMesh()->UnlinkAnimClassLayers(OverrideAnimClass);
		OverrideAnimInstance.Reset();
	}
}
