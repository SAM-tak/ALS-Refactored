// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterTasks/AlsOverrideTask.h"
#include "AlsCharacter.h"
#include "AlsCharacterTaskAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverrideTask)

void UAlsOverrideTask::Begin()
{
	if (!bActive && IsValid(OverrideAnimClass))
	{
		OverrideAnimInstance = Cast<UAlsCharacterTaskAnimInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverrideAnimClass));

		if (!OverrideAnimInstance.IsValid())
		{
			Character->GetMesh()->LinkAnimClassLayers(OverrideAnimClass);
			OverrideAnimInstance = Cast<UAlsCharacterTaskAnimInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverrideAnimClass));
		}

		OverrideAnimInstance->Refresh(this);
	}
	Super::Begin();
}

void UAlsOverrideTask::Refresh(float DeltaTime)
{
	Super::Refresh(DeltaTime);
	if (OverrideAnimInstance.IsValid())
	{
		OverrideAnimInstance->Refresh(this);
	}
}

void UAlsOverrideTask::OnFinished()
{
	if (Character.IsValid() && IsValid(OverrideAnimClass))
	{
		Character->GetMesh()->UnlinkAnimClassLayers(OverrideAnimClass);
		OverrideAnimInstance.Reset();
	}

	Super::OnFinished();
}
