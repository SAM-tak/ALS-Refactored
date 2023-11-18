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

	// Speed of root bone
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Meta = (ForceUnits = "cm/s"))
	float RootBoneSpeed{0.0f};

	// Highest speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Meta = (ForceUnits = "cm/s"))
	float MaxBoneSpeed{0.0f};

	// Highest angular speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Meta = (ForceUnits = "deg"))
	float MaxBoneAngularSpeed{0.0f};
};
