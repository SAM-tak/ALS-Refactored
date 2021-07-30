﻿#pragma once

#include "AlsAimingCharacterState.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsAimingCharacterState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator SmoothRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float YawSpeed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = -180, ClampMax = 180))
	float PreviousSmoothYawAngle{0.0f};
};
