// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility_Montage.h"
#include "AlsGameplayAbility.generated.h"

class AAlsCharacter;

/**
 *
 */
UCLASS()
class ALS_API UAlsGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "ALS|Ability")
    AAlsCharacter* GetAlsCharacterFromActorInfo() const;
};
