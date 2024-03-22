#include "Utility/AlsGameplayTags.h"

namespace AlsTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als")
}

namespace AlsDesiredStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Desired")
}

namespace AlsDesiredViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Desired.ViewMode")
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, "Als.Desired.ViewMode.FirstPerson")
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, "Als.Desired.ViewMode.ThirdPerson")
}

namespace AlsDesiredRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Desired.RotationMode")
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, "Als.Desired.RotationMode.VelocityDirection")
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, "Als.Desired.RotationMode.ViewDirection")
	UE_DEFINE_GAMEPLAY_TAG(Aiming, "Als.Desired.RotationMode.Aiming")
}

namespace AlsDesiredStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Desired.Stance")
	UE_DEFINE_GAMEPLAY_TAG(Standing, "Als.Desired.Stance.Standing")
	UE_DEFINE_GAMEPLAY_TAG(Crouching, "Als.Desired.Stance.Crouching")
	UE_DEFINE_GAMEPLAY_TAG(LyingFront, "Als.Desired.Stance.LyingFront")
	UE_DEFINE_GAMEPLAY_TAG(LyingBack, "Als.Desired.Stance.LyingBack")
}

namespace AlsDesiredGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Desired.Gait")
	UE_DEFINE_GAMEPLAY_TAG(Walking, "Als.Desired.Gait.Walking")
	UE_DEFINE_GAMEPLAY_TAG(Running, "Als.Desired.Gait.Running")
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, "Als.Desired.Gait.Sprinting")
}

namespace AlsViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.ViewMode")
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, "Als.ViewMode.FirstPerson")
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, "Als.ViewMode.ThirdPerson")
}

namespace AlsLocomotionModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.LocomotionMode")
	UE_DEFINE_GAMEPLAY_TAG(Grounded, "Als.LocomotionMode.Grounded")
	UE_DEFINE_GAMEPLAY_TAG(InAir, "Als.LocomotionMode.InAir")
}

namespace AlsRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.RotationMode")
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, "Als.RotationMode.VelocityDirection")
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, "Als.RotationMode.ViewDirection")
	UE_DEFINE_GAMEPLAY_TAG(Aiming, "Als.RotationMode.Aiming")
}

namespace AlsAimingModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.AimingMode")
	UE_DEFINE_GAMEPLAY_TAG(AimDownSight, "Als.AimingMode.AimDownSight")
	UE_DEFINE_GAMEPLAY_TAG(HipFire, "Als.AimingMode.HipFire")
}

namespace AlsStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Stance")
	UE_DEFINE_GAMEPLAY_TAG(Standing, "Als.Stance.Standing")
	UE_DEFINE_GAMEPLAY_TAG(Crouching, "Als.Stance.Crouching")
	UE_DEFINE_GAMEPLAY_TAG(LyingFront, "Als.Stance.LyingFront")
	UE_DEFINE_GAMEPLAY_TAG(LyingBack, "Als.Stance.LyingBack")
}

namespace AlsGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Gait")
	UE_DEFINE_GAMEPLAY_TAG(Walking, "Als.Gait.Walking")
	UE_DEFINE_GAMEPLAY_TAG(Running, "Als.Gait.Running")
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, "Als.Gait.Sprinting")
}

namespace AlsOverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.OverlayMode")
	UE_DEFINE_GAMEPLAY_TAG(Default, "Als.OverlayMode.Default")
	UE_DEFINE_GAMEPLAY_TAG(Masculine, "Als.OverlayMode.Masculine")
	UE_DEFINE_GAMEPLAY_TAG(Feminine, "Als.OverlayMode.Feminine")
	UE_DEFINE_GAMEPLAY_TAG(Injured, "Als.OverlayMode.Injured")
	UE_DEFINE_GAMEPLAY_TAG(HandsTied, "Als.OverlayMode.HandsTied")
	UE_DEFINE_GAMEPLAY_TAG(M4, "Als.OverlayMode.M4")
	UE_DEFINE_GAMEPLAY_TAG(PistolOneHanded, "Als.OverlayMode.PistolOneHanded")
	UE_DEFINE_GAMEPLAY_TAG(PistolTwoHanded, "Als.OverlayMode.PistolTwoHanded")
	UE_DEFINE_GAMEPLAY_TAG(Bow, "Als.OverlayMode.Bow")
	UE_DEFINE_GAMEPLAY_TAG(Torch, "Als.OverlayMode.Torch")
	UE_DEFINE_GAMEPLAY_TAG(Binoculars, "Als.OverlayMode.Binoculars")
	UE_DEFINE_GAMEPLAY_TAG(Box, "Als.OverlayMode.Box")
	UE_DEFINE_GAMEPLAY_TAG(Barrel, "Als.OverlayMode.Barrel")
}

namespace AlsLocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.LocomotionAction")
	UE_DEFINE_GAMEPLAY_TAG(Mantling, "Als.LocomotionAction.Mantling")
	UE_DEFINE_GAMEPLAY_TAG(Vaulting, "Als.LocomotionAction.Vaulting")
	UE_DEFINE_GAMEPLAY_TAG(GettingUp, "Als.LocomotionAction.GettingUp")
	UE_DEFINE_GAMEPLAY_TAG(GettingDown, "Als.LocomotionAction.GettingDown")
	UE_DEFINE_GAMEPLAY_TAG(Rolling, "Als.LocomotionAction.Rolling")
	UE_DEFINE_GAMEPLAY_TAG(Landing, "Als.LocomotionAction.Landing")
	UE_DEFINE_GAMEPLAY_TAG(FreeFalling, "Als.LocomotionAction.FreeFalling")
	UE_DEFINE_GAMEPLAY_TAG(BeingKnockedDown, "Als.LocomotionAction.BeingKnockedDown")
	UE_DEFINE_GAMEPLAY_TAG(Dying, "Als.LocomotionAction.Dying")
}

namespace AlsStateFlagTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.StateFlag")
	UE_DEFINE_GAMEPLAY_TAG(LeftShoulder, "Als.StateFlag.LeftShoulder")
	UE_DEFINE_GAMEPLAY_TAG(FacingUpward, "Als.StateFlag.FacingUpward")
	UE_DEFINE_GAMEPLAY_TAG(MantleHigh, "Als.StateFlag.MantleHigh")
	UE_DEFINE_GAMEPLAY_TAG(MantleMedium, "Als.StateFlag.MantleMedium")
	UE_DEFINE_GAMEPLAY_TAG(MantleLow, "Als.StateFlag.MantleLow")
}

namespace AlsGroundedEntryModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.GroundedEntryMode")
	UE_DEFINE_GAMEPLAY_TAG(StandingToCrouching, "Als.GroundedEntryMode.StandingToCrouching")
	UE_DEFINE_GAMEPLAY_TAG(CrouchingToStanding, "Als.GroundedEntryMode.CrouchingToStanding")
	UE_DEFINE_GAMEPLAY_TAG(LyingFrontToCrouching, "Als.GroundedEntryMode.LyingFrontToCrouching")
	UE_DEFINE_GAMEPLAY_TAG(LyingBackToCrouching, "Als.GroundedEntryMode.LyingBackToCrouching")
	UE_DEFINE_GAMEPLAY_TAG(CrouchingToLyingFront, "Als.GroundedEntryMode.CrouchingToLyingFront")
	UE_DEFINE_GAMEPLAY_TAG(CrouchingToLyingBack, "Als.GroundedEntryMode.CrouchingToLyingBack")
}
