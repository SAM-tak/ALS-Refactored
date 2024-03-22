// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterTasks/AlsRagdollingTask.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsCharacterTaskAnimInstance.h"
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

	Super::Refresh(DeltaTime);
	
	if (!IsActive() || RagdollingState.bFreezing)
	{
		return;
	}

	if (RagdollingState.IsGroundedAndAged())
	{
		if (!bOnGroundedAndAgedFired)
		{
			bOnGroundedAndAgedFired = true;
			K2_OnGroundedAndAged();
		}
		Character->Lie();

		// local only. not be replicated.
		Character->GetAlsAbilitySystem()->SetLooseGameplayTagCount(AlsStateFlagTags::FacingUpward, RagdollingState.bFacingUpward ? 1 : 0);
	}
	else
	{
		bOnGroundedAndAgedFired = false;
	}
}

bool UAlsRagdollingTask::IsEpilogRunning_Implementation() const
{
	if (OverrideAnimInstance.IsValid())
	{
		bool CharacterTaskActive{OverrideAnimInstance->GetCharacterTaskActive()};
		float BlendWeight{OverrideAnimInstance->GetObservingFinalBlendWeight()};
		//UE_LOG(LogTemp, Log, TEXT("bActive:%d ObservingFinalBlendWeight:%0.2f (%d)"), CharacterTaskActive, BlendWeight,
		//	   CharacterTaskActive || (0.f < BlendWeight && BlendWeight < 1.f));
		return CharacterTaskActive || (0.f < BlendWeight && BlendWeight < 1.f);
	}
	return false;
}

bool UAlsRagdollingTask::IsGroundedAndAged() const
{
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};
	return RagdollingState.IsGroundedAndAged();
}
