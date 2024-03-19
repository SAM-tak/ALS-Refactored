// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Actions/AlsGameplayAbility_Ragdolling.h"
#include "Abilities/Tasks/AlsAbilityTask_Tick.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAnimationInstance.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsPhysicalAnimationComponent.h"
#include "LinkedAnimLayers/AlsRagdollingAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AlsOverrideModeComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Ragdolling)

UAlsGameplayAbility_Ragdolling::UAlsGameplayAbility_Ragdolling(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AbilityTags.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Root);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
}

void UAlsGameplayAbility_Ragdolling::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsActive())
	{
		TickTask = UAlsAbilityTask_Tick::New(this, FName(TEXT("UAlsGameplayAbility_Ragdolling")));
		if (TickTask.IsValid())
		{
			TickTask->OnTick.AddDynamic(this, &ThisClass::Tick);
			TickTask->ReadyForActivation();
		}
	}
}

void UAlsGameplayAbility_Ragdolling::Tick(const float DeltaTime)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};

	if (!IsActive() || RagdollingState.bFreezing)
	{
		return;
	}

	K2_OnTick(DeltaTime);

	if (IsGroundedAndAged())
	{
		if (!bOnGroundedAndAgedFired)
		{
			bOnGroundedAndAgedFired = true;
			K2_OnGroundedAndAged();
		}
	}
	else
	{
		bOnGroundedAndAgedFired = false;
	}
}

void UAlsGameplayAbility_Ragdolling::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};

	//if (RagdollingState.IsGroundedAndAged())
	//{
	//	auto* AbilitySystem{GetAlsAbilitySystemComponentFromActorInfo()};
	//	AbilitySystem->SetLooseGameplayTagCount(AlsStateFlagTags::FacingUpward, RagdollingState.bFacingUpward ? 1 : 0);
	//}

	auto* OverrideModeComponent{Character->GetComponentByClass<UAlsOverrideModeComponent>()};
	OverrideModeComponent->EndCurrentRagdollingTask();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UAlsGameplayAbility_Ragdolling::IsGroundedAndAged() const
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};
	return RagdollingState.IsGroundedAndAged();
}
