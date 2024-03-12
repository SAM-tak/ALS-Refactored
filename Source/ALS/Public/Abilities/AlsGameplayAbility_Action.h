// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/AlsGameplayAbility.h"
#include "AlsGameplayAbility_Action.generated.h"

class AAlsCharacter;
class UAlsAbilitySystemComponent;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Action : public UAlsGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
