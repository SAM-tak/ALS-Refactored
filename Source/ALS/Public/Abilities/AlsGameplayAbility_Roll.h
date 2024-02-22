// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AlsGameplayAbility_Montage.h"
#include "AlsGameplayAbility_Roll.generated.h"

/**
 * Roll Action
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Roll : public UAlsGameplayAbility_Montage
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability|Roll")
	uint8 bCrouchOnStart : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability|Roll")
	uint8 bRotateToInputOnStart : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability|Roll", Meta = (ClampMin = 0))
	float RotationInterpolationSpeed{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability|Roll")
	uint8 bInterruptRollingWhenInAir : 1{true};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|Roll|State", Transient, Meta = (ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Als|Ability|Roll")
	float CalcTargetYawAngle() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Als|Ability|Roll")
	void TickOnRoll(const float DeltaTime);

private:
	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;
	FDelegateHandle PhysicsRotationHandle;

	UFUNCTION()
	void ProcessTick(const float DeltaTime);

	void RefreshRolling(const float DeltaTime);
};
