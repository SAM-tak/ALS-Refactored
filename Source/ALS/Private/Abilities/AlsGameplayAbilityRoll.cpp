// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/AlsGameplayAbilityRoll.h"
#include "Utility/AlsGameplayTags.h"
#include "AlsCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbilityRoll)

UAlsGameplayAbilityRoll::UAlsGameplayAbilityRoll(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityTags.AddTag(AlsLocomotionActionTags::Rolling);
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::Rolling);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::GettingUp);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Mantling);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Ragdolling);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Rolling);
}

bool UAlsGameplayAbilityRoll::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		const auto* Character{Cast<AAlsCharacter>(ActorInfo->OwnerActor)};
		return IsValid(Character) && Character->HasMatchingGameplayTag(AlsLocomotionModeTags::Grounded);
	}
	return false;
}

void UAlsGameplayAbilityRoll::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	TargetYawAngle = bRotateToInputOnStart && Character->GetLocomotionState().bHasInput
		? Character->GetLocomotionState().InputYawAngle
		: UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw));

	Character->RefreshRotationInstantOnRolling(TargetYawAngle);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAlsGameplayAbilityRoll::OnMontageEnded_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	Super::OnMontageEnded_Implementation(Montage, bInterrupted);
}
