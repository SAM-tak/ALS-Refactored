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

	if (!bFreezed)
	{
		// Save a snapshot of the current ragdoll pose for use in animation graph to blend out of the ragdoll.
		if (GetSkelMeshComponent()->GetNumComponentSpaceTransforms() > 0) // When stop PIE, SnapshotPose rises Out of range exception.
		{
			SnapshotPose(FinalPose);
		}

		bFreezed = true;
	}
}

void UAlsRagdollingAnimInstance::UnFreeze()
{
	check(IsInGameThread())

	if (bFreezed)
	{
		bFreezed = false;
	}
}

void UAlsRagdollingAnimInstance::Refresh(const UAlsGameplayAbility_Ragdolling& Ability)
{
	check(IsInGameThread())

	bActive = Ability.IsActive();
	StartBlendTime = Ability.StartBlendTime;
	bGroundedAndAged = Ability.IsGroundedAndAged();
	bFacingUpward = Ability.bFacingUpward;
	LyingDownYawAngleDelta = Ability.LyingDownYawAngleDelta;
}
