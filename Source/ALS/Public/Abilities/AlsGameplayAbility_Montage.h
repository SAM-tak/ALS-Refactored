// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "Abilities/AlsGameplayAbility.h"
#include "AlsGameplayAbility_Montage.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;

/**
 *	A gameplay ability that plays a single montage and applies a GameplayEffect
 */
UCLASS()
class ALS_API UAlsGameplayAbility_Montage : public UAlsGameplayAbility
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	float PlayRate{1.0f};

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	FName SectionName;

	/** GameplayEffects to apply and then remove while the animation is playing */
	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffectClassesWhileAnimating;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	uint8 EndsAbilityOnMontageEnded : 1{true};

private:
	TArray<struct FActiveGameplayEffectHandle> AppliedEffects;

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintNativeEvent, Category = AlsMontageAbility)
	void OnMontageEnded(UAnimMontage *Montage, bool bInterrupted);

	void GetGameplayEffectsWhileAnimating(TArray<const UGameplayEffect *> &OutEffects) const;
};
