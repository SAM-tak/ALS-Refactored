#pragma once

#include "Engine/DataAsset.h"
#include "AlsCameraGameplayTags.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 0.99))
	float FirstPersonFactorThreshold{0.7f};

	// Threshold of Aiming Amount value for aim down sight.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 0.99))
	float ADSThreshold{0.9f};

	// If bLeftDominantEye is true, use LeftEyeCameraSocketName instead of RightEyeCameraSocketName.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bLeftDominantEye : 1 {false};

	// The distance to move backward from the camera sokcet position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float RetreatDistance{10.0f};

	// Initial Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FGameplayTagContainer RecoilStateTags{AlsCameraTags::Recoiling};
};

USTRUCT(BlueprintType)
struct ALSCAMERA_API FAlsThirdPersonCameraStateSettings
{
	GENERATED_BODY()

	// Initial Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FGameplayTag ShoulderMode{AlsCameraShoulderModeTags::Right};

	// If greater than zero, camera location same as FPP when distance from third person camera pivot by blocking by geometry less than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float AutoFPPStartDistance{80.0f};

	// Ends Auto FPP when distance from third person camera pivot by blocking by geometry greater than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float AutoFPPEndDistance{100.0f};

	// The horizontal field of view (in degrees) in panoramic rendering.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocusTraceStartOffset{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> TraceChannel{ECC_Visibility};
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
