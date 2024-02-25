// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
//#include "UObject/ObjectMacros.h"
//#include "Templates/SubclassOf.h"
//#include "GameplayAbilitySpec.h"
//#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "AlsGameplayAbility.generated.h"

class AAlsCharacter;
class UAlsAbilitySystemComponent;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "ALS|Ability")
    AAlsCharacter* GetAlsCharacterFromActorInfo() const;

    UFUNCTION(BlueprintPure, Category = "ALS|Ability")
    UAlsAbilitySystemComponent* GetAlsAbilitySystemComponentFromActorInfo() const;
};
