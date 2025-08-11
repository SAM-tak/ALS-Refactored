#pragma once

#include "Engine/DataAsset.h"
#include "Engine/Scene.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsCameraGameplayTags.h"
#include "AlsGameplayCameraStateSettings.generated.h"

class UCurveFloat;

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsFirstPersonCameraStateSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName CameraSocketName{TEXTVIEW("FirstPersonCamera")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName LeftEyeCameraSocketName{TEXTVIEW("ADSCameraLeft")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FName RightEyeCameraSocketName{TEXTVIEW("ADSCameraRight")};

	// Threshold of Aiming Amount value for aim down sight.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 0.99))
	float ADSThreshold{0.9f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS")
	TSubclassOf<UCameraShakeBase> ADSCameraShakeClass{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 1))
	float ADSCameraShakeScale{0.2f};

	// If bLeftDominantEye is true, use LeftEyeCameraSocketName instead of RightEyeCameraSocketName.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bLeftDominantEye : 1 {false};

	// The distance to move backward from the camera sokcet position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float RetreatDistance{10.0f};

	// The warp threshold distance in trasitioning to first person view from third person view.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float HeadSize{30.0f};
};

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsThirdPersonCameraStateSettings
{
	GENERATED_BODY()

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

	// Initial Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FGameplayTag ShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (InlineEditConditionToggle))
	uint8 bEnableTraceDistanceSmoothing : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bEnableTraceDistanceSmoothing"))
	float TraceDistanceInterpolationSpeed{3.0f};

	// If greater than zero, camera location same as FPP when distance from third person camera pivot by blocking by geometry less than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float AutoFPPStartDistance{80.0f};

	// Ends Auto FPP when distance from third person camera pivot by blocking by geometry greater than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float AutoFPPEndDistance{100.0f};

	// The horizontal field of view (in degrees) in panoramic rendering.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocusTraceStartOffset{10.0f};
};

UCLASS(Blueprintable, BlueprintType)
class ALSCAMERA_API UAlsGameplayCameraStateSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FGameplayTag DesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsFirstPersonCameraStateSettings FirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsThirdPersonCameraStateSettings ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "s"))
	float ViewModeChangeBlockTime{0.08f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TEnumAsByte<ECollisionChannel> FocusTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float MinFocalLength{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float MaxFocalLength{5000.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocusTraceRadius{3.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<UCurveFloat> HeuristicPitchMapping{nullptr};
};
