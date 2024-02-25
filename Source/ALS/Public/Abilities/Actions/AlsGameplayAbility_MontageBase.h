// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "Abilities/AlsGameplayAbility.h"
#include "AlsGameplayAbility_MontageBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAlsMontageNotifyDelegate, FName, NotifyName, float, TriggerTime, float, Duration);

class UAbilitySystemComponent;
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
	float StartTime;
};

/**
 *	A gameplay ability that plays a single montage and applies a GameplayEffect
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_MontageBase : public UAlsGameplayAbility
{
	GENERATED_UCLASS_BODY()

protected:
	/** GameplayEffects to apply and then remove while the animation is playing */
	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility)
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffectClassesWhileAnimating;

	UPROPERTY(EditDefaultsOnly, Category = AlsMontageAbility, Meta = (ForceUnit = "s"))
	float BlendOutDurationOnCancel{0.15f};

	UPROPERTY(BlueprintAssignable)
	FAlsMontageNotifyDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable)
	FAlsMontageNotifyDelegate OnNotifyEnd;

	UPROPERTY(VisibleAnywhere, Category = "AlsMontageAbility|State", Transient)
	float CurrentMotangeDuration{0.f};

	UPROPERTY(VisibleAnywhere, Category = "AlsMontageAbility|State", Transient)
	int32 CurrentMontageInstanceID{INDEX_NONE};

private:
	TArray<struct FActiveGameplayEffectHandle> AppliedEffects;

public:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintNativeEvent, Category = AlsMontageAbility)
	void OnEndMontage(UAnimMontage *Montage, bool bInterrupted);

	void GetGameplayEffectsWhileAnimating(TArray<const UGameplayEffect *> &OutEffects) const;

protected:
	void PlayMontage(const FGameplayAbilityActivationInfo ActivationInfo, const FAlsPlayMontageParameter& Parameter,
					 const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);

	void PlayMontage(const FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float PlayRate, FName SectionName, float StartTime,
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo);

	UFUNCTION(BlueprintCallable, Category = AlsMontageAbility)
	void StopMontage(float OverrideBlendOutTime = -1.0f);

private:
	UFUNCTION(BlueprintCallable, Category = AlsMontageAbility)
	void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName SectionName = NAME_None, float StartTime = 0.0f);

	bool IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload) const;

	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload);

	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload);
};
