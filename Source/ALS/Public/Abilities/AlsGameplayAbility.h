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
class UAnimMontage;

USTRUCT(BlueprintType)
struct ALS_API FAlsPlayMontageParameter
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	float PlayRate{ 1.0f };

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	FName SectionName;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility, Meta = (ForceUnit = "s"))
	float StartTime{ 0.0f };
};

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

protected:
	virtual void PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo, const FAlsPlayMontageParameter& Parameter);

	virtual void PlayMontage(const FAlsPlayMontageParameter& Parameter);

	void StopCurrentMontage(const FGameplayAbilityActorInfo* ActorInfo, float OverrideBlendOutTime = -1.0f);

	UFUNCTION(BlueprintCallable, Category = AlsMontageAbility)
	void StopCurrentMontage(float OverrideBlendOutTime = -1.0f);
};
