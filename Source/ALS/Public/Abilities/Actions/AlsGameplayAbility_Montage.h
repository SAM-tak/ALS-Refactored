// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/Actions/AlsGameplayAbility_MontageBase.h"
#include "AlsGameplayAbility_Montage.generated.h"

/**
 *	A gameplay ability that plays a single montage and applies a GameplayEffect
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Montage : public UAlsGameplayAbility_MontageBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	float PlayRate{1.0f};

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	FName SectionName;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility, Meta = (ForceUnit = "s"))
	float StartTime;


public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
