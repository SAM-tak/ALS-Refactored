// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/AlsGameplayAbility.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
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

void UAlsGameplayAbility::PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo,
									  const FAlsPlayMontageParameter& Parameter)
{
	auto* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();

	if (Parameter.MontageToPlay && AbilitySystemComponent)
	{
		AbilitySystemComponent->PlayMontage(this, ActivationInfo, Parameter.MontageToPlay, Parameter.PlayRate, Parameter.SectionName, Parameter.StartTime);
	}
}

void UAlsGameplayAbility::PlayMontage(const FAlsPlayMontageParameter& Parameter)
{
	PlayMontage(GetCurrentActivationInfo(), GetCurrentActorInfo(), Parameter);
}

void UAlsGameplayAbility::StopCurrentMontage(const FGameplayAbilityActorInfo* ActorInfo, float OverrideBlendOutTime)
{
	auto* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CurrentMontageStop(OverrideBlendOutTime);
	}
}

void UAlsGameplayAbility::StopCurrentMontage(float OverrideBlendOutTime)
{
	StopCurrentMontage(GetCurrentActorInfo(), OverrideBlendOutTime);
}

void UAlsGameplayAbility::SetGameplayTag(const FGameplayTag& Tag) const
{
	GetAlsCharacterFromActorInfo()->GetAlsAbilitySystem()->SetLooseGameplayTagCount(Tag, 1);
}

void UAlsGameplayAbility::ResetGameplayTag(const FGameplayTag& Tag) const
{
	GetAlsCharacterFromActorInfo()->GetAlsAbilitySystem()->SetLooseGameplayTagCount(Tag, 0);
}

void UAlsGameplayAbility::AddGameplayTag(const FGameplayTag& Tag) const
{
	GetAlsCharacterFromActorInfo()->GetAlsAbilitySystem()->AddLooseGameplayTag(Tag);
}

void UAlsGameplayAbility::SubtractGameplayTag(const FGameplayTag& Tag) const
{
	GetAlsCharacterFromActorInfo()->GetAlsAbilitySystem()->RemoveLooseGameplayTag(Tag);
}

void UAlsGameplayAbility::K2_SetGameplayTag(FGameplayTag Tag) const
{
	SetGameplayTag(Tag);
}

void UAlsGameplayAbility::K2_ResetGameplayTag(FGameplayTag Tag) const
{
	ResetGameplayTag(Tag);
}

void UAlsGameplayAbility::K2_AddGameplayTag(FGameplayTag Tag) const
{
	AddGameplayTag(Tag);
}

void UAlsGameplayAbility::K2_SubtractGameplayTag(FGameplayTag Tag) const
{
	SubtractGameplayTag(Tag);
}

void UAlsGameplayAbility::SetInputBlocked(bool bBlocked) const
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetAlsCharacterMovement()->SetInputBlocked(bBlocked);
	}
}
