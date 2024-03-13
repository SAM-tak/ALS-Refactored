// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "Utility/AlsGameplayTags.h"
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
	float PlayRate{1.0f};

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	FName SectionName;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility, Meta = (ForceUnit = "s"))
	float StartTime{0.0f};
};

/**
 * GameplayAbility for ALS
 * Can bind Input action
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	// Bind input actions on activate this ability and unbind when finished. (default : true)
	// If activates too frequency and no needs to input binding, turn off for better performance.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AlsAbility)
	uint8 bEnableInputBinding : 1{true};

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Ability")
	AAlsCharacter* GetAlsCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Ability")
	UAlsAbilitySystemComponent* GetAlsAbilitySystemComponentFromActorInfo() const;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo, const FAlsPlayMontageParameter& Parameter);

	virtual void PlayMontage(const FAlsPlayMontageParameter& Parameter);

	void StopCurrentMontage(const FGameplayAbilityActorInfo* ActorInfo, float OverrideBlendOutTime = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Montage")
	void StopCurrentMontage(float OverrideBlendOutTime = -1.0f);

	void SetGameplayTag(const FGameplayTag& Tag) const;

	void ResetGameplayTag(const FGameplayTag& Tag) const;

	void AddGameplayTag(const FGameplayTag& Tag) const;

	void SubtractGameplayTag(const FGameplayTag& Tag) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Set GameplayTag", ScriptName = "SetGameplayTag"))
	void K2_SetGameplayTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Reset GameplayTag", ScriptName = "ResetGameplayTag"))
	void K2_ResetGameplayTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Add GameplayTag", ScriptName = "AddGameplayTag"))
	void K2_AddGameplayTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Subtract GameplayTag", ScriptName = "SubtractGameplayTag"))
	void K2_SubtractGameplayTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability")
	void SetInputBlocked(bool bBlocked) const;
};
