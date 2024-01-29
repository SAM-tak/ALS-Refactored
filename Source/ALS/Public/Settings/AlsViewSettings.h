#pragma once

#include "AlsViewSettings.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsViewSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	float LookRotationInterpSpeed{15.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	float AdjustControllRotationSpeed{15.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS")
	uint8 bEnableNetworkSmoothing : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS")
	uint8 bEnableListenServerNetworkSmoothing : 1 {true};
};
