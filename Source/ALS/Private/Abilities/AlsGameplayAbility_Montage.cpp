// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/AlsGameplayAbility_Montage.h"
#include "AlsAbilitySystemComponent.h"
#include "Animation/AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Montage)

// --------------------------------------------------------------------------------------------------------------------------------------------------------
//
//	UAlsGameplayAbility_Montage
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------

void UAlsGameplayAbility_Montage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
												  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData *TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	PlayMontage(MontageToPlay, Handle, ActorInfo, ActivationInfo);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CurrentMotangeDuration <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, false);
	}
}
