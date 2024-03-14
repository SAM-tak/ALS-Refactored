#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "AlsGroundedAnimInstance.generated.h"

// Grounded Linked Anim Layer
// Tag : "Grounded"
UCLASS(Abstract, AutoExpandCategories = ("ALS|Settings"))
class ALS_API UAlsGroundedAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|State", Transient)
	FGameplayTag GroundedEntryMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|State", Transient)
	float StartPositionOverrideForGroundedEntry{0.0f};

	// External reference in AnimBP :
	// CurrentGameplayTags
	// LocomotionState.bMovingSmooth
	// RotateInPlaceState.bRotatingLeft
	// RotateInPlaceState.bRotatingRight
	// PoseState.StandingAmount
	// PoseState.CrouchingAmount

	// Access to GroundedState : None

public:
	void SetGroundedEntryMode(const FGameplayTag& NewGroundedEntryMode, float STartPosition);

protected:
	UFUNCTION(BlueprintPure, Category = "ALS|Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	FGameplayTagContainer GetEntryStance() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ResetGroundedEntryMode();
};

inline void UAlsGroundedAnimInstance::SetGroundedEntryMode(const FGameplayTag& NewGroundedEntryMode, float NewStartPosition)
{
	GroundedEntryMode = NewGroundedEntryMode;
	StartPositionOverrideForGroundedEntry = NewStartPosition;
}

inline void UAlsGroundedAnimInstance::ResetGroundedEntryMode()
{
	GroundedEntryMode = FGameplayTag::EmptyTag;
}
