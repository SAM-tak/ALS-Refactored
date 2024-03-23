// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/AlsGameplayAbility_Action.h"
#include "AlsCharacter.h"
#include "AlsAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Action)

void UAlsGameplayAbility_Action::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	auto* Character{GetAlsCharacterFromActorInfo()};
	if (Character)
	{
		auto& LocomotionMode{Character->GetLocomotionMode()};
		if (LocomotionMode == AlsLocomotionModeTags::Grounded)
		{
			auto& DesiredStance{Character->GetDesiredStance()};
			if (DesiredStance == AlsDesiredStanceTags::Standing)
			{
				Character->UnCrouch();
			}
			else if (DesiredStance == AlsDesiredStanceTags::Crouching)
			{
				Character->Crouch();
			}
		}
		else if (LocomotionMode == AlsLocomotionModeTags::InAir)
		{
			Character->UnCrouch();
		}
	}
}
