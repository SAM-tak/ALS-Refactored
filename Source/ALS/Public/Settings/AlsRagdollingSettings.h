#pragma once

#include "Engine/EngineTypes.h"
#include "AlsRagdollingSettings.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct ALS_API FAlsRagdollingSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bStartRagdollingOnLand{true};

	// Ragdolling will start if the character lands with a speed greater than the specified value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
		Meta = (ClampMin = 0, EditCondition = "bStartRagdollingOnLand", ForceUnits = "cm/s"))
	float RagdollingOnLandSpeedThreshold{1000.0f};

	// If checked, the ragdoll's speed will be limited by the character's last speed for a few frames
	// after activation. This hack is used to prevent the ragdoll from getting a very high initial speed
	// at unstable FPS, which can be reproduced by jumping and activating the ragdoll at the same time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bLimitInitialRagdollSpeed{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "x"))
	float StrengthMultiplier{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "s"))
	float StrengthMultiplierBlendTime{0.15f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bAllowFreeze{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedForForceFreezing{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedAndStoppedForForceFreezing{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float RootBoneSpeedConsideredAsStopped{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float SpeedThresholdToFreeze{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "deg"))
	float AngularSpeedThresholdToFreeze{45.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TArray<TEnumAsByte<ECollisionChannel>> GroundTraceResponseChannels;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS", AdvancedDisplay)
	FCollisionResponseContainer GroundTraceResponses{ECR_Ignore};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TObjectPtr<UAnimMontage> GetUpFrontMontage{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TObjectPtr<UAnimMontage> GetUpBackMontage{nullptr};

public:
#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
