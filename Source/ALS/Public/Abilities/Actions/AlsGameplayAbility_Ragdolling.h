// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/AlsGameplayAbility_Action.h"
#include "AlsGameplayAbility_Ragdolling.generated.h"

/**
 * Ragdolling
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Ragdolling : public UAlsGameplayAbility_Action
{
	GENERATED_UCLASS_BODY()

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	uint8 bOnGroundedAndAgedFired : 1{false};

	UFUNCTION(BlueprintPure, Category = "ALS|Ability|Ragdolling")
	bool IsGroundedAndAged() const;

	UFUNCTION()
	void Tick(const float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Ability|Ragdolling", DisplayName = "Tick", Meta = (ScriptName = "Tick"))
	void K2_OnTick(const float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Ability|Ragdolling", DisplayName = "On Grounded And Aged", Meta = (ScriptName = "OnGroundedAndAged"))
	void K2_OnGroundedAndAged();

	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;
};
