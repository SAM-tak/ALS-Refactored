#pragma once

#include "Utility/AlsGameplayTags.h"
#include "AlsGameplayTagSettings.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsGameplayTagSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer DesiredState{AlsDesiredStateTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer DesiredViewModes{AlsDesiredViewModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer DesiredRotationModes{AlsDesiredRotationModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer DesiredStances{AlsDesiredStanceTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer DesiredGaits{AlsDesiredGaitTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer LocomotionModes{AlsLocomotionModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer ViewModes{AlsViewModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer RotationModes{AlsRotationModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer AimingModes{AlsAimingModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer Stances{AlsStanceTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer Gaits{AlsGaitTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer OverlayModes{AlsOverlayModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FGameplayTagContainer Actions{AlsLocomotionActionTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	TMap<FGameplayTag, FGameplayTag> DesiredToActualMap{
		{AlsDesiredViewModeTags::Root, AlsViewModeTags::Root},
		{AlsDesiredViewModeTags::FirstPerson, AlsViewModeTags::FirstPerson},
		{AlsDesiredViewModeTags::ThirdPerson, AlsViewModeTags::ThirdPerson},
		{AlsDesiredRotationModeTags::Root, AlsRotationModeTags::Root},
		{AlsDesiredRotationModeTags::VelocityDirection, AlsRotationModeTags::VelocityDirection},
		{AlsDesiredRotationModeTags::ViewDirection, AlsRotationModeTags::ViewDirection},
		{AlsDesiredRotationModeTags::Aiming, AlsRotationModeTags::Aiming},
		{AlsDesiredStanceTags::Root, AlsStanceTags::Root},
		{AlsDesiredStanceTags::Standing, AlsStanceTags::Standing},
		{AlsDesiredStanceTags::Crouching, AlsStanceTags::Crouching},
		{AlsDesiredStanceTags::Lying, AlsStanceTags::Lying},
		{AlsDesiredGaitTags::Root, AlsGaitTags::Root},
		{AlsDesiredGaitTags::Walking, AlsGaitTags::Walking},
		{AlsDesiredGaitTags::Running, AlsGaitTags::Running},
		{AlsDesiredGaitTags::Sprinting, AlsGaitTags::Sprinting},
	};
};
