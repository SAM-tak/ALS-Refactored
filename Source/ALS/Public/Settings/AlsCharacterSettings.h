#pragma once

#include "AlsInAirRotationMode.h"
#include "AlsViewSettings.h"
#include "Utility/AlsGameplayTags.h"
#include "AlsCharacterSettings.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ALS_API UAlsCharacterSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold{50.0f};

	// When FirstPerson Or RotateToVelocityWhenSprinting is False And DesiredRotationMode is not VelocityDirection,
	// sprint will allow if View Relative Angle less than this value. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float ViewRelativeAngleThresholdForSprint{50.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	EAlsInAirRotationMode InAirRotationMode{EAlsInAirRotationMode::KeepRelativeRotation};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bAllowAimingWhenInAir : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bSprintHasPriorityOverAiming : 1{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bRotateToVelocityWhenSprinting : 1{true};

	// If checked, the character will rotate relative to the object it is standing on in the velocity
	// direction rotation mode, otherwise the character will ignore that object and keep its world rotation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bInheritMovementBaseRotationInVelocityDirectionRotationMode : 1{false};

	// If checked, the character will rotate towards the direction they want to move, but is not always able to due to obstacles.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bAutoTurnOffSprint : 1{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsViewSettings View;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GameplayTag")
	FGameplayTagContainer OverlayModeTags{AlsOverlayModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GameplayTag")
	FGameplayTagContainer ActionTags{AlsLocomotionActionTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GameplayTag")
	TMap<FGameplayTag, FGameplayTag> DesiredToActualMap{
		{AlsDesiredViewModeTags::FirstPerson, AlsViewModeTags::FirstPerson},
		{AlsDesiredViewModeTags::ThirdPerson, AlsViewModeTags::ThirdPerson},
		{AlsDesiredRotationModeTags::VelocityDirection, AlsRotationModeTags::VelocityDirection},
		{AlsDesiredRotationModeTags::ViewDirection, AlsRotationModeTags::ViewDirection},
		{AlsDesiredRotationModeTags::Aiming, AlsRotationModeTags::Aiming},
		{AlsDesiredStanceTags::Standing, AlsStanceTags::Standing},
		{AlsDesiredStanceTags::Crouching, AlsStanceTags::Crouching},
		{AlsDesiredStanceTags::LyingFront, AlsStanceTags::LyingFront},
		{AlsDesiredStanceTags::LyingBack, AlsStanceTags::LyingBack},
		{AlsDesiredGaitTags::Walking, AlsGaitTags::Walking},
		{AlsDesiredGaitTags::Running, AlsGaitTags::Running},
		{AlsDesiredGaitTags::Sprinting, AlsGaitTags::Sprinting},
	};
};
