#pragma once

#include "Engine/EngineTypes.h"
#include "AlsRagdollingSettings.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ALS_API UAlsRagdollingSettings : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TArray<TEnumAsByte<ECollisionChannel>> GroundTraceResponseChannels{ECC_WorldStatic, ECC_WorldDynamic, ECC_Destructible};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	FCollisionResponseContainer GroundTraceResponses{ECR_Ignore};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "s"))
	float StartBlendTime{0.25f};

	// If checked, it stops the physical simulation and returns control of the bone to kinematic
	// when the conditions mentioned later are met.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	uint8 bAllowFreeze : 1{false};

	// The time until it freezes forcibly after landing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedForForceFreezing{5.0f};

	// The time until it forcibly freezes after the root bone is considered to have stopped when landing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedAndStoppedForForceFreezing{1.0f};

	// When the speed is below this value, the root bone is considered to be stopped.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float RootBoneSpeedConsideredAsStopped{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float SpeedThresholdToFreeze{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "deg"))
	float AngularSpeedThresholdToFreeze{45.0f};

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
