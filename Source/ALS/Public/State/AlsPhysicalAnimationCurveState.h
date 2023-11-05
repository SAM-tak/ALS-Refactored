#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationCurveState.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationCurveState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftHand{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightHand{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float FreeLeftLeg{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float FreeRightLeg{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float FreeNeck{0.0f};
};
