#include "LinkedAnimLayers/AlsGroundedAnimInstance.h"
#include "AlsAnimationInstance.h"
#include "AlsAnimationInstanceProxy.h"
#include "AlsCharacter.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGroundedAnimInstance)

FGameplayTagContainer UAlsGroundedAnimInstance::GetEntryStance() const
{
	if (GroundedEntryMode == AlsGroundedEntryModeTags::StandingToCrouching)
	{
		return FGameplayTagContainer{AlsStanceTags::Standing};
	}
	if (GroundedEntryMode == AlsGroundedEntryModeTags::CrouchingToStanding ||
		GroundedEntryMode == AlsGroundedEntryModeTags::CrouchingToLyingFront ||
		GroundedEntryMode == AlsGroundedEntryModeTags::CrouchingToLyingBack)
	{
		return FGameplayTagContainer{AlsStanceTags::Crouching};
	}
	if (GroundedEntryMode == AlsGroundedEntryModeTags::LyingFrontToCrouching)
	{
		return FGameplayTagContainer{AlsStanceTags::LyingFront};
	}
	if (GroundedEntryMode == AlsGroundedEntryModeTags::LyingBackToCrouching)
	{
		return FGameplayTagContainer{AlsStanceTags::LyingBack};
	}
	if (Parent.IsValid())
	{
		return FGameplayTagContainer{Parent->GetCurrentGameplayTags().Filter(FGameplayTagContainer{AlsStanceTags::Root}).First()};
	}
	return FGameplayTagContainer{AlsStanceTags::Standing};
}
