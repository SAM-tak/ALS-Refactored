#pragma once

#include "AlsInAirRotationMode.h"
#include "AlsMantlingSettings.h"
#include "AlsRagdollingSettings.h"
#include "AlsRollingSettings.h"
#include "AlsViewSettings.h"
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
	uint8 bAllowAimingWhenInAir : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bSprintHasPriorityOverAiming : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bRotateToVelocityWhenSprinting : 1 {true};

	// If checked, the character will rotate relative to the object it is standing on in the velocity
	// direction rotation mode, otherwise the character will ignore that object and keep its world rotation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bInheritMovementBaseRotationInVelocityDirectionRotationMode : 1 {false};

	// If checked, the character will rotate towards the direction they want to move, but is not always able to due to obstacles.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAutoTurnOffSprint{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsViewSettings View;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsGeneralMantlingSettings Mantling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsRagdollingSettings Ragdolling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsRollingSettings Rolling;

public:
	UAlsCharacterSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
