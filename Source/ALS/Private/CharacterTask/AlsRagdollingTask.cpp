// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterTasks/AlsRagdollingTask.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAnimationInstance.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsPhysicalAnimationComponent.h"
#include "LinkedAnimLayers/AlsRagdollingAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsRagdollingTask)

void UAlsRagdollingTask::Refresh(float DeltaTime)
{
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};

	if (!IsActive() || RagdollingState.bFreezing)
	{
		return;
	}

	Super::Refresh(DeltaTime);

	if (RagdollingState.IsGroundedAndAged())
	{
		if (!bOnGroundedAndAgedFired)
		{
			bOnGroundedAndAgedFired = true;
			K2_OnGroundedAndAged();
		}
		Character->Lie();

		if (RagdollingState.bFacingUpward)
		{
			Character->LocalTags.AddTag(AlsStateFlagTags::FacingUpward);
		}
		else
		{
			Character->LocalTags.RemoveTag(AlsStateFlagTags::FacingUpward);
		}
	}
	else
	{
		bOnGroundedAndAgedFired = false;
	}
}

bool UAlsRagdollingTask::IsGroundedAndAged() const
{
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};
	return RagdollingState.IsGroundedAndAged();
}