// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AlsGameplayAbility_Rolling.h"
#include "AlsGameplayAbility_Landing.generated.h"

/**
 * Landing
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Landing : public UAlsGameplayAbility_Rolling
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bStartRagdollingOnLand : 1 {true};

	// Ragdolling will start if the character lands with a speed greater than the specified value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bStartRagdollingOnLand", ForceUnits = "cm/s"))
	float RagdollingOnLandSpeedThreshold{1000.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bStartRollingOnLand : 1 {true};

	// Rolling will start if the character lands with a speed greater than the specified value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, EditCondition = "bStartRollingOnLand", ForceUnits = "cm/s"))
	float RollingOnLandSpeedThreshold{700.0f};

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual float CalcTargetYawAngle_Implementation() const override;
};
