#pragma once

#include "AlsCharacterTaskAnimInstance.h"
#include "AlsOverlayAnimInstance.generated.h"

UCLASS(Abstract, AutoExpandCategories = ("ALS|Settings"))
class ALS_API UAlsOverlayAnimInstance : public UAlsCharacterTaskAnimInstance
{
	GENERATED_BODY()

public:
	// utility for overlays. overlay accesses View->PitchAmount
	UFUNCTION(BlueprintPure, Category = "ALS|Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	UAlsViewAnimInstance* GetViewUnsafe() const;

	// External aceess:
	// PoseState
	// IdleAdditiveAmount
	// CurrentGameplayTags
};
