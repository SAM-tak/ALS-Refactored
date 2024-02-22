// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/AlsGameplayAbility_Landing.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsMath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Landing)

UAlsGameplayAbility_Landing::UAlsGameplayAbility_Landing(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityTags.Reset();
	AbilityTags.AddTag(AlsLocomotionActionTags::Landing);
	ActivationOwnedTags.Reset();
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::Landing);
	BlockAbilitiesWithTag.Reset();
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Landing);
}

float UAlsGameplayAbility_Landing::CalcTargetYawAngle_Implementation() const
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	return Character->GetLocomotionState().bHasSpeed
		   ? Character->GetLocomotionState().VelocityYawAngle
		   : UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw));
}

bool UAlsGameplayAbility_Landing::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
													 const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
													 OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		const auto* Character{Cast<AAlsCharacter>(ActorInfo->OwnerActor)};
		if (bStartRagdollingOnLand && Character->GetLocomotionState().Velocity.Z <= -RagdollingOnLandSpeedThreshold)
		{
			return true;
		}
		else if (bStartRollingOnLand && Character->GetLocomotionState().Velocity.Z <= -RollingOnLandSpeedThreshold)
		{
			return true;
		}
	}
	return false;
}

void UAlsGameplayAbility_Landing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	if (bStartRagdollingOnLand && Character->GetLocomotionState().Velocity.Z <= -RagdollingOnLandSpeedThreshold)
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			return;
		}

		Character->StartRagdolling();

		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
				   ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, false);
	}
	else if (bStartRollingOnLand && Character->GetLocomotionState().Velocity.Z <= -RollingOnLandSpeedThreshold)
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
}
