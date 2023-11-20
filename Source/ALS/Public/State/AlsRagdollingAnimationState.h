﻿#pragma once

#include "Animation/PoseSnapshot.h"
#include "AlsRagdollingAnimationState.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsRagdollingAnimationState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "deg"))
	float LyingDownYawAngleDelta{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FPoseSnapshot FinalRagdollPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "x"))
	float FlailPlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bGroundedAndAged{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bFacingUpward{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bFreezed{false};
};
