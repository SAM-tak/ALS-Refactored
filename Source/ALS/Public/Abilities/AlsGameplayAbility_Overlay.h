// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/AlsGameplayAbility.h"
#include "AlsGameplayAbility_Overlay.generated.h"

class UAlsOverlayAnimInstance;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Overlay : public UAlsGameplayAbility
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
