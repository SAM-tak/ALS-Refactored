// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/AlsGameplayAbility_Action.h"
#include "AlsGameplayAbility_MontageBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAlsMontageNotifyDelegate, FName, NotifyName, float, TriggerTime, float, Duration);

class UAbilitySystemComponent;

/**
 *	A gameplay ability that plays a single montage and applies a GameplayEffect
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_MontageBase : public UAlsGameplayAbility_Action
{
	GENERATED_BODY()

protected:
	/** GameplayEffects to apply and then remove while the animation is playing */
	UPROPERTY(EditDefaultsOnly, Category = "AlsAbility|Montage")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffectClassesWhileAnimating;

	UPROPERTY(EditDefaultsOnly, Category = "AlsAbility|Montage", Meta = (ForceUnit = "s"))
	float BlendOutDurationOnCancel{0.15f};

	UPROPERTY(BlueprintAssignable, Category = "AlsAbility|Montage")
	FAlsMontageNotifyDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable, Category = "AlsAbility|Montage")
	FAlsMontageNotifyDelegate OnNotifyEnd;

	UPROPERTY(VisibleAnywhere, Category = "AlsAbility|Montage|State", Transient)
	float CurrentMotangeDuration{0.f};

	UPROPERTY(VisibleAnywhere, Category = "AlsAbility|Montage|State", Transient)
	int32 CurrentMontageInstanceId{INDEX_NONE};

private:
	TArray<struct FActiveGameplayEffectHandle> AppliedEffects;

protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Montage")
	void OnEndMontage(UAnimMontage *Montage, bool bInterrupted);

	void SetUpNotification(UAnimInstance* AnimInstance, UAnimMontage* Montage);

	void GetGameplayEffectsWhileAnimating(TArray<const UGameplayEffect *> &OutEffects) const;

	bool PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FAlsPlayMontageParameter& Parameter,
					 const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);

	bool PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, UAnimMontage* Montage, float PlayRate, FName SectionName, float StartTime,
					 const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);

	virtual bool PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo,
							 const FAlsPlayMontageParameter& Parameter) override;

	virtual bool PlayMontage(const FAlsPlayMontageParameter& Parameter) override;

private:
	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Montage")
	bool PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName SectionName = NAME_None, float StartTime = 0.0f);

	bool IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload) const;

	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload);

	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload);
};
