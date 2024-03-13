#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "AlsRagdollingAnimInstance.generated.h"

UCLASS(Abstract)
class ALS_API UAlsRagdollingAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsRagdolling, Transient)
	FPoseSnapshot FinalPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsRagdolling, Transient, Meta = (ForceUnits = "deg"))
	float LyingDownYawAngleDelta{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsRagdolling, Transient, Meta = (ClampMin = 0, ForceUnits = "s"))
	float StartBlendTime{0.3f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsRagdolling, Transient)
	uint8 bActive : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsRagdolling, Transient)
	uint8 bGroundedAndAged : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsRagdolling, Transient)
	uint8 bFacingUpward : 1{false};

public:
	FPoseSnapshot& GetFinalPoseSnapshot();

	void Freeze();

	void UnFreeze();

	void Refresh(const class UAlsGameplayAbility_Ragdolling& Ability, bool bNewActive);
};
