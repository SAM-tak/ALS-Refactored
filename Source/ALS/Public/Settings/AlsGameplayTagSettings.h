#pragma once

#include "Utility/AlsGameplayTags.h"
#include "AlsGameplayTagSettings.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsGameplayTagSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FGameplayTagContainer OverlayModes{AlsOverlayModeTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FGameplayTagContainer Actions{AlsLocomotionActionTags::Root};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TMap<FGameplayTag, FGameplayTag> DesiredToActualMap{
		{AlsDesiredViewModeTags::FirstPerson, AlsViewModeTags::FirstPerson},
		{AlsDesiredViewModeTags::ThirdPerson, AlsViewModeTags::ThirdPerson},
		{AlsDesiredRotationModeTags::VelocityDirection, AlsRotationModeTags::VelocityDirection},
		{AlsDesiredRotationModeTags::ViewDirection, AlsRotationModeTags::ViewDirection},
		{AlsDesiredRotationModeTags::Aiming, AlsRotationModeTags::Aiming},
		{AlsDesiredStanceTags::Standing, AlsStanceTags::Standing},
		{AlsDesiredStanceTags::Crouching, AlsStanceTags::Crouching},
		{AlsDesiredStanceTags::Lying, AlsStanceTags::Lying},
		{AlsDesiredGaitTags::Walking, AlsGaitTags::Walking},
		{AlsDesiredGaitTags::Running, AlsGaitTags::Running},
		{AlsDesiredGaitTags::Sprinting, AlsGaitTags::Sprinting},
	};
};
