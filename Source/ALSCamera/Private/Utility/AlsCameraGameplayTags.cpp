#include "Utility/AlsCameraGameplayTags.h"

namespace AlsCameraTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Camera")
}

namespace AlsCameraViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Camera.ViewMode")
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, "Als.Camera.ViewMode.FirstPerson")
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, "Als.Camera.ViewMode.ThirdPerson")
}

namespace AlsCameraShoulderModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Als.Camera.ShoulderMode")
	UE_DEFINE_GAMEPLAY_TAG(Center, "Als.Camera.ShoulderMode.Center")
	UE_DEFINE_GAMEPLAY_TAG(Left, "Als.Camera.ShoulderMode.Left")
	UE_DEFINE_GAMEPLAY_TAG(Right, "Als.Camera.ShoulderMode.Right")
}
