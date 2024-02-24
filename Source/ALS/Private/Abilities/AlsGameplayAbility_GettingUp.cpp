// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/AlsGameplayAbility_GettingUp.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Utility/AlsGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_GettingUp)

UAlsGameplayAbility_GettingUp::UAlsGameplayAbility_GettingUp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	AbilityTags.AddTag(AlsLocomotionActionTags::GettingUp);
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::GettingUp);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Root);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::GettingUp);
}

bool UAlsGameplayAbility_GettingUp::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
													   const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
													   OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		const auto* Character{Cast<AAlsCharacter>(ActorInfo->OwnerActor)};
		return IsValid(Character) && Character->HasMatchingGameplayTag(AlsLocomotionModeTags::Grounded);
	}
	return false;
}

void UAlsGameplayAbility_GettingUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
													const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	auto* Character{GetAlsCharacterFromActorInfo()};
	
	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetAlsCharacterMovement()->SetInputBlocked(true);
	}

	auto MontageToPlay{Character->HasMatchingGameplayTag(AlsStateFlagTags::FacingUpward) ? GetUpBackMontage : GetUpFrontMontage};

	PlayMontage(MontageToPlay, Handle, ActorInfo, ActivationInfo);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CurrentMotangeDuration <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, false);
	}
}

void UAlsGameplayAbility_GettingUp::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	auto* Character{GetAlsCharacterFromActorInfo()};

	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetAlsCharacterMovement()->SetInputBlocked(false);
	}
}
