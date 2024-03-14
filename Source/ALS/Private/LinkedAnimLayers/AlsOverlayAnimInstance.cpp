#include "LinkedAnimLayers/AlsOverlayAnimInstance.h"
#include "AlsAnimationInstance.h"
#include "AlsCharacter.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverlayAnimInstance)

UAlsViewAnimInstance* UAlsOverlayAnimInstance::GetViewUnsafe() const
{
	return Parent->GetViewAnimInstance();
}
