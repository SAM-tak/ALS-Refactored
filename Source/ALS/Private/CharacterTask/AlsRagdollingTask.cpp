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

UAlsRagdollingTask::UAlsRagdollingTask(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GroundTraceResponses.WorldStatic = ECR_Block;
	GroundTraceResponses.WorldDynamic = ECR_Block;
	GroundTraceResponses.Destructible = ECR_Block;
}

void UAlsRagdollingTask::Begin()
{
	Super::Begin();
}

void UAlsRagdollingTask::Refresh(float DeltaTime)
{
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};

	if (!IsActive() || RagdollingState.bFreezing)
	{
		return;
	}

	Super::Refresh(DeltaTime);

	if (IsGroundedAndAged())
	{
		if (!bOnGroundedAndAgedFired)
		{
			bOnGroundedAndAgedFired = true;
			K2_OnGroundedAndAged();
		}
		Character->Lie();
	}
	else
	{
		bOnGroundedAndAgedFired = false;
	}
}

void UAlsRagdollingTask::OnEnd(bool bWasCancelled)
{
	Super::OnEnd(bWasCancelled);

	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};

	if (RagdollingState.IsGroundedAndAged())
	{
		auto* AbilitySystem{Character->GetAlsAbilitySystem()};
		AbilitySystem->SetLooseGameplayTagCount(AlsStateFlagTags::FacingUpward, RagdollingState.bFacingUpward ? 1 : 0);
	}
}

bool UAlsRagdollingTask::IsGroundedAndAged() const
{
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};
	return RagdollingState.IsGroundedAndAged();
}

UAlsLinkedAnimationInstance* UAlsRagdollingTask::GetOverrideAnimInstance() const
{
	auto* PhysicalAnimation{Character->GetPhysicalAnimation()};
	auto& RagdollingState{PhysicalAnimation->GetRagdollingState()};
	return RagdollingState.OverrideAnimInstance.Get();
}

#if WITH_EDITOR
void UAlsRagdollingTask::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(FAlsRagdollingSettings, GroundTraceResponseChannels))
	{
		return;
	}

	GroundTraceResponses.SetAllChannels(ECR_Ignore);

	for (const auto& CollisionChannel : GroundTraceResponseChannels)
	{
		GroundTraceResponses.SetResponse(CollisionChannel, ECR_Block);
	}
}
#endif
