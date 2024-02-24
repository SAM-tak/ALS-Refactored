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
