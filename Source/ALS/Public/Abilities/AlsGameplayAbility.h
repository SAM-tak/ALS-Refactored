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
	GENERATED_UCLASS_BODY()

	friend class UAlsAbilitySystemComponent;

protected:
	// Bind input actions on activate this ability and unbind when finished. (default : true)
	// If activates too frequency and no needs to input binding, turn off for better performance.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AlsAbility)
	uint8 bEnableInputBinding : 1{true};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AlsAbility)
	uint8 bStopCurrentMontageOnEndAbility : 1{false};

	// A minus value means not override (default)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AlsAbility)
	float OverrideBlendOutTimeOnEndAbility{-1.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	uint8 bInputBinded : 1{false};

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Ability")
	AAlsCharacter* GetAlsCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Ability")
	UAlsAbilitySystemComponent* GetAlsAbilitySystemComponentFromActorInfo() const;

	virtual UWorld* GetWorld() const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo, const FAlsPlayMontageParameter& Parameter);

	virtual bool PlayMontage(const FAlsPlayMontageParameter& Parameter);

	void StopCurrentMontage(const FGameplayAbilityActorInfo* ActorInfo, float OverrideBlendOutTime = -1.0f) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Montage")
	void StopCurrentMontage(float OverrideBlendOutTime = -1.0f) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability")
	void SetInputBlocked(bool bBlocked) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|MotionWarping")
	void AddOrUpdateWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|MotionWarping")
	void AddOrUpdateWarpTarget(const FMotionWarpingTarget& WarpTarget);

	virtual void OnControllerChanged(AController* PreviousController, AController* NewController);

	void BindInput(UInputComponent* InputComponent);

	void UnbindInput(UInputComponent* InputComponent);
};
