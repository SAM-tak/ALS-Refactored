#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "AlsRagdollingAnimInstance.generated.h"

// Ragdolling Linked Anim Layer
// Tag : "Ragdolling"
UCLASS(Abstract)
class ALS_API UAlsRagdollingAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	FPoseSnapshot FinalPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ForceUnits = "deg"))
	float LyingDownYawAngleDelta{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ForceUnits = "s"))
	float StartBlendTime{0.3f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	uint8 bActive : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	uint8 bGroundedAndAged : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	uint8 bFacingUpward : 1{false};

public:
	FPoseSnapshot& GetFinalPoseSnapshot();

	void Freeze();

	void UnFreeze();

	void SetStartBlendTime(float NewStartBlendTime);

	void Refresh(const struct FAlsRagdollingState& State, bool bNewActive);
};

inline void UAlsRagdollingAnimInstance::SetStartBlendTime(float NewStartBlendTime)
{
	StartBlendTime = NewStartBlendTime;
}
