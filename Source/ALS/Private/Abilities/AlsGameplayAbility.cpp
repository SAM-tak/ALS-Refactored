// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/AlsGameplayAbility.h"
#include "AlsCharacter.h"
#include "AlsAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility)

AAlsCharacter* UAlsGameplayAbility::GetAlsCharacterFromActorInfo() const
{
	if (!ensure(CurrentActorInfo))
	{
		return nullptr;
	}
    return Cast<AAlsCharacter>(CurrentActorInfo->AvatarActor.Get());
}

UAlsAbilitySystemComponent* UAlsGameplayAbility::GetAlsAbilitySystemComponentFromActorInfo() const
{
	auto* AlsCharacter{GetAlsCharacterFromActorInfo()};
    return AlsCharacter ? AlsCharacter->GetAlsAbilitySystem() : nullptr;
}

void UAlsGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (Character)
	{
		auto LocomotionMode{Character->GetLocomotionMode()};
		if (LocomotionMode == AlsLocomotionModeTags::Grounded)
		{
			auto DesiredStance{Character->GetDesiredStance()};
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
