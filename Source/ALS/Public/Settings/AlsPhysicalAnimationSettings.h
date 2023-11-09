#pragma once

#include "Engine/EngineTypes.h"
#include "AlsPhysicalAnimationSettings.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnActivate{0.05f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnDeactivate{0.3f};

	/** Place the clavicle bone under the influence of physical animation, not just in ragdoll but also in regular physical animation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bIsAffectedClavicle{true};

	/** Apply the constraint profile with the same name as the physical animation profile if it exists. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bUseConstraintProfile{false};
};
