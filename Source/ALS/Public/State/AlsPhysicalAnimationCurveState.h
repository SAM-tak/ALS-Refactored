#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationCurveState.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationCurveState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftArm{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightArm{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftHand{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightHand{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftLeg{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightLeg{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftFoot{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightFoot{0.0f};
};
