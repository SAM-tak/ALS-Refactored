#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationCurveState.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationCurveState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float HitReaction{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float Idle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float Mantle{0.0f};
};
