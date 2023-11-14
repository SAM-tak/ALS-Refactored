#pragma once

#include "AlsRagdollingState.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsRagdollingState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bGrounded{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bFacingUpward{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "s"))
	float ElapsedTime{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "s"))
	float TimeAfterGrounded{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "s"))
	float TimeAfterGroundedAndStopped{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bFreezing{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Meta = (ForceUnits = "cm/s"))
	float RootBoneSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Meta = (ForceUnits = "cm/s"))
	float MaxBoneSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Meta = (ForceUnits = "deg"))
	float MaxBoneAngularSpeed{0.0f};
};
