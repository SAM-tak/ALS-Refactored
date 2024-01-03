#pragma once

#include "Engine/DataAsset.h"
#include "Engine/Scene.h"
#include "Utility/AlsConstants.h"
#include "AlsCameraSettings.generated.h"

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsFirstPersonCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 5, ClampMax = 170, ForceUnits = "deg"))
	float FOV{90.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName CameraSocketName{TEXTVIEW("FirstPersonCamera")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName AimingCameraLeftSocketName{TEXTVIEW("AimingFirstPersonCamera_l")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName AimingCameraRightSocketName{TEXTVIEW("AimingFirstPersonCamera_r")};

	// The distance to move backward from the camera sokcet position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float RetreatDistance{10.0f};

	// The warp threshold distance in trasitioning to first person view from third person view.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float HeadSize{30.0f};

	// The dominant eye setting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bLeftDominantEye : 1 {false};

	// If bPanoramic is true, renders panoramic with partial multi-view.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bPanoramic : 1 {false};

	// The horizontal field of view (in degrees) in panoramic rendering.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 180, ClampMax = 360, ForceUnits = "deg"))
	float PanoramaFOV{180.0f};

	/**
	 * This specifies the proportion of the side view within the range of 0 to 1.
	 * A value of 0 means no side view, and a value of 1 means the side view takes up one third of the entire screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 1))
	float PanoramaSideViewRate{0.5f};
};

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsTraceDistanceSmoothingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	float InterpolationSpeed{3.0f};
};

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsThirdPersonCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 5, ClampMax = 170, ForceUnits = "deg"))
	float FOV{90.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName FirstPivotSocketName{UAlsConstants::RootBoneName()};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName SecondPivotSocketName{UAlsConstants::HeadBoneName()};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float TraceRadius{15.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> TraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName TraceShoulderLeftSocketName{TEXTVIEW("ThirdPersonTraceShoulderLeft")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName TraceShoulderRightSocketName{TEXTVIEW("ThirdPersonTraceShoulderRight")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FVector3f TraceOverrideOffset{0.0f, 0.0f, 40.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (InlineEditConditionToggle))
	uint8 bEnableTraceDistanceSmoothing : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
		DisplayName = "Enable Trace Distance Smoothing", Meta = (EditCondition = "bEnableTraceDistanceSmoothing"))
	FAlsTraceDistanceSmoothingSettings TraceDistanceSmoothing;

	// If greater than zero, camera location same as FPP when distance from third person camera pivot by blocking by geometry less than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float AutoFPPStartDistance{80.0f};

	// Ends Auto FPP when distance from third person camera pivot by blocking by geometry greater than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float AutoFPPEndDistance{100.0f};

	// If bPanoramic is true, renders panoramic with partial multi-view.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bApplyVelocityLead : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "x"))
	FVector VelocityLeadRate{0.0f, 0.0f, 0.0f};

	// The horizontal field of view (in degrees) in panoramic rendering.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	float VelocityLeadInterpSpeed{10.0f};

	// If bPanoramic is true, renders panoramic with partial multi-view.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bPanoramic : 1 {false};

	// The horizontal field of view (in degrees) in panoramic rendering.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 180, ClampMax = 360, ForceUnits = "deg"))
	float PanoramaFOV{180.0f};

	/**
	 * This specifies the proportion of the side view within the range of 0 to 1.
	 * A value of 0 means no side view, and a value of 1 means the side view takes up one third of the entire screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 1))
	float PanoramaSideViewRate{0.5f};
};

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsCameraLagSubsteppingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0.005, ClampMax = 0.5, ForceUnits = "s"))
	float LagSubstepDeltaTime{1.0f / 60.0f};
};

UCLASS(Blueprintable, BlueprintType)
class ALSCAMERA_API UAlsCameraSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bIgnoreTimeDilation : 1 {true};

	// The camera will be teleported if the character has moved further than this
	// distance in 1 frame. If zero is specified, then teleportation will be disabled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float TeleportDistanceThreshold{200.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsFirstPersonCameraSettings FirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsThirdPersonCameraSettings ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (InlineEditConditionToggle))
	uint8 bEnableCameraLagSubstepping : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", DisplayName = "Enable Camera Lag Substepping",
		Meta = (EditCondition = "bEnableCameraLagSubstepping"))
	FAlsCameraLagSubsteppingSettings CameraLagSubstepping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FPostProcessSettings PostProcess;

public:
#if WITH_EDITORONLY_DATA
	virtual void Serialize(FArchive& Archive) override;
#endif
};
