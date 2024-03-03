#include "AlsRagdollingAnimInstance.h"
#include "Abilities/Actions/AlsGameplayAbility_Ragdolling.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsRagdollingAnimInstance)

FPoseSnapshot& UAlsRagdollingAnimInstance::GetFinalPoseSnapshot()
{
	check(IsInGameThread())

	return FinalPose;
}

void UAlsRagdollingAnimInstance::Freeze()
{
	check(IsInGameThread())

	if (!FinalPose.bIsValid)
	{
		// Save a snapshot of the current ragdoll pose for use in animation graph to blend out of the ragdoll.
		if (GetSkelMeshComponent()->GetNumComponentSpaceTransforms() > 0) // When stop PIE, SnapshotPose rises Out of range exception.
		{
			SnapshotPose(FinalPose);
		}
	}
}

void UAlsRagdollingAnimInstance::UnFreeze()
{
	check(IsInGameThread())

	if (FinalPose.bIsValid)
	{
		FinalPose.Reset();
	}
}

void UAlsRagdollingAnimInstance::Refresh(const UAlsGameplayAbility_Ragdolling& Ability, bool bNewActive)
{
	check(IsInGameThread())

	//bActive = Ability.IsActive() && !Ability.bIsAbilityEnding; // this is not work. bIsAbilityEnding can be true in Super::EndAbility.
	bActive = bNewActive;
	StartBlendTime = Ability.StartBlendTime;
	bGroundedAndAged = Ability.IsGroundedAndAged();
	bFacingUpward = Ability.bFacingUpward;
	LyingDownYawAngleDelta = Ability.LyingDownYawAngleDelta;
}
