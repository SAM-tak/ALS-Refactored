// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AlsGameplayAbility_Montage.h"
#include "AlsGameplayAbilityRoll.generated.h"

/**
 * 
 */
UCLASS()
class ALS_API UAlsGameplayAbilityRoll : public UAlsGameplayAbility_Montage
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	uint8 bCrouchOnStart : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	uint8 bRotateToInputOnStart : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0))
	float RotationInterpolationSpeed{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	uint8 bStartRollingOnLand : 1{true};

	// Rolling will start if the character lands with a speed greater than the specified value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability",
		Meta = (ClampMin = 0, EditCondition = "bStartRollingOnLand", ForceUnits = "cm/s"))
	float RollingOnLandSpeedThreshold{700.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	uint8 bInterruptRollingWhenInAir : 1{true};

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

public:
	UAlsGameplayAbilityRoll();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void OnMontageEnded_Implementation(UAnimMontage* Montage, bool bInterrupted) override;
};
