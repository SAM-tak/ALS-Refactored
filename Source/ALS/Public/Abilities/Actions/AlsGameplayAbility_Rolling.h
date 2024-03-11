// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Actions/AlsGameplayAbility_Montage.h"
#include "AlsGameplayAbility_Rolling.generated.h"

/**
 * Rolling Action
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Rolling : public UAlsGameplayAbility_Montage
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling")
	uint8 bCrouchOnStart : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling")
	uint8 bRotateToInputOnStart : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling", Meta = (ClampMin = 0))
	float RotationInterpolationSpeed{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling")
	uint8 bCancelRollingWhenInAir : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling", Meta = (EditCondition = bCancelRollingWhenInAir, ForceUnits = "s"))
	float TimeToCancel{0.3f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling", Meta = (EditCondition = bCancelRollingWhenInAir))
	FGameplayTag TryActiveWhenCancel{AlsLocomotionActionTags::FreeFalling};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling", Meta = (ClampMin = 0, ClampMax = 1))
	float StartPositionOfStandingEntry{0.6f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling", Meta = (ClampMin = 0, ClampMax = 1))
	float StartPositionOfCrouchingEntry{0.3f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling|State", Transient, Meta = (ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|Rolling|State", Transient, Meta = (ForceUnits = "s"))
	float InAirTime{0.0f};

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Rolling")
	float CalcTargetYawAngle() const;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Rolling")
	void Tick(const float DeltaTime);

private:
	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;

	FDelegateHandle PhysicsRotationHandle;

	void RefreshRolling(const float DeltaTime);
};
