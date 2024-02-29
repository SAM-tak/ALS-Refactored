#include "Utility/AlsGameplayTags.h"

namespace AlsTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als")})
}

namespace AlsDesiredStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{ TEXTVIEW("Als.Desired") })
}

namespace AlsDesiredViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.Desired.ViewMode")})
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, FName{TEXTVIEW("Als.Desired.ViewMode.FirstPerson")})
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, FName{TEXTVIEW("Als.Desired.ViewMode.ThirdPerson")})
}

namespace AlsDesiredRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.Desired.RotationMode")})
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, FName{TEXTVIEW("Als.Desired.RotationMode.VelocityDirection")})
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, FName{TEXTVIEW("Als.Desired.RotationMode.ViewDirection")})
	UE_DEFINE_GAMEPLAY_TAG(Aiming, FName{TEXTVIEW("Als.Desired.RotationMode.Aiming")})
}

namespace AlsDesiredStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.Desired.Stance")})
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{TEXTVIEW("Als.Desired.Stance.Standing")})
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{TEXTVIEW("Als.Desired.Stance.Crouching")})
	UE_DEFINE_GAMEPLAY_TAG(Lying, FName{TEXTVIEW("Als.Desired.Stance.Lying")})
}

namespace AlsDesiredGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.Desired.Gait")})
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName{TEXTVIEW("Als.Desired.Gait.Walking")})
	UE_DEFINE_GAMEPLAY_TAG(Running, FName{TEXTVIEW("Als.Desired.Gait.Running")})
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName{TEXTVIEW("Als.Desired.Gait.Sprinting")})
}

namespace AlsViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.ViewMode")})
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, FName{TEXTVIEW("Als.ViewMode.FirstPerson")})
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, FName{TEXTVIEW("Als.ViewMode.ThirdPerson")})
}

namespace AlsLocomotionModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.LocomotionMode")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{TEXTVIEW("Als.LocomotionMode.Grounded")})
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{TEXTVIEW("Als.LocomotionMode.InAir")})
}

namespace AlsRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.RotationMode")})
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, FName{TEXTVIEW("Als.RotationMode.VelocityDirection")})
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, FName{TEXTVIEW("Als.RotationMode.ViewDirection")})
	UE_DEFINE_GAMEPLAY_TAG(Aiming, FName{TEXTVIEW("Als.RotationMode.Aiming")})
}

namespace AlsAimingModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.AimingMode")})
	UE_DEFINE_GAMEPLAY_TAG(AimDownSight, FName{TEXTVIEW("Als.AimingMode.AimDownSight")})
	UE_DEFINE_GAMEPLAY_TAG(HipFire, FName{TEXTVIEW("Als.AimingMode.HipFire")})
}

namespace AlsStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.Stance")})
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{TEXTVIEW("Als.Stance.Standing")})
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{TEXTVIEW("Als.Stance.Crouching")})
	UE_DEFINE_GAMEPLAY_TAG(Lying, FName{TEXTVIEW("Als.Stance.Lying")})
}

namespace AlsGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.Gait")})
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName{TEXTVIEW("Als.Gait.Walking")})
	UE_DEFINE_GAMEPLAY_TAG(Running, FName{TEXTVIEW("Als.Gait.Running")})
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName{TEXTVIEW("Als.Gait.Sprinting")})
}

namespace AlsOverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.OverlayMode")})
	UE_DEFINE_GAMEPLAY_TAG(Default, FName{TEXTVIEW("Als.OverlayMode.Default")})
	UE_DEFINE_GAMEPLAY_TAG(Masculine, FName{TEXTVIEW("Als.OverlayMode.Masculine")})
	UE_DEFINE_GAMEPLAY_TAG(Feminine, FName{TEXTVIEW("Als.OverlayMode.Feminine")})
	UE_DEFINE_GAMEPLAY_TAG(Injured, FName{TEXTVIEW("Als.OverlayMode.Injured")})
	UE_DEFINE_GAMEPLAY_TAG(HandsTied, FName{TEXTVIEW("Als.OverlayMode.HandsTied")})
	UE_DEFINE_GAMEPLAY_TAG(M4, FName{TEXTVIEW("Als.OverlayMode.M4")})
	UE_DEFINE_GAMEPLAY_TAG(PistolOneHanded, FName{TEXTVIEW("Als.OverlayMode.PistolOneHanded")})
	UE_DEFINE_GAMEPLAY_TAG(PistolTwoHanded, FName{TEXTVIEW("Als.OverlayMode.PistolTwoHanded")})
	UE_DEFINE_GAMEPLAY_TAG(Bow, FName{TEXTVIEW("Als.OverlayMode.Bow")})
	UE_DEFINE_GAMEPLAY_TAG(Torch, FName{TEXTVIEW("Als.OverlayMode.Torch")})
	UE_DEFINE_GAMEPLAY_TAG(Binoculars, FName{TEXTVIEW("Als.OverlayMode.Binoculars")})
	UE_DEFINE_GAMEPLAY_TAG(Box, FName{TEXTVIEW("Als.OverlayMode.Box")})
	UE_DEFINE_GAMEPLAY_TAG(Barrel, FName{TEXTVIEW("Als.OverlayMode.Barrel")})
}

namespace AlsLocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.LocomotionAction")})
	UE_DEFINE_GAMEPLAY_TAG(Mantling, FName{TEXTVIEW("Als.LocomotionAction.Mantling")})
	UE_DEFINE_GAMEPLAY_TAG(Ragdolling, FName{TEXTVIEW("Als.LocomotionAction.Ragdolling")})
	UE_DEFINE_GAMEPLAY_TAG(GettingUp, FName{TEXTVIEW("Als.LocomotionAction.GettingUp")})
	UE_DEFINE_GAMEPLAY_TAG(Rolling, FName{TEXTVIEW("Als.LocomotionAction.Rolling")})
	UE_DEFINE_GAMEPLAY_TAG(Landing, FName{TEXTVIEW("Als.LocomotionAction.Landing")})
	UE_DEFINE_GAMEPLAY_TAG(FreeFalling, FName{TEXTVIEW("Als.LocomotionAction.FreeFalling")})
}

namespace AlsStateFlagTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.StateFlag")})
	UE_DEFINE_GAMEPLAY_TAG(FacingUpward, FName{TEXTVIEW("Als.StateFlag.FacingUpward")})
}

namespace AlsGroundedEntryModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, FName{TEXTVIEW("Als.GroundedEntryMode")})
	UE_DEFINE_GAMEPLAY_TAG(StandingToCrouching, FName{TEXTVIEW("Als.GroundedEntryMode.StandingToCrouching")})
	UE_DEFINE_GAMEPLAY_TAG(CrouchingToStanding, FName{TEXTVIEW("Als.GroundedEntryMode.CrouchingToStanding")})
}	 // namespace AlsGroundedEntryModeTags
