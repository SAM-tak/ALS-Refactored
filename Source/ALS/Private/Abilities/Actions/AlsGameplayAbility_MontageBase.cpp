// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/Actions/AlsGameplayAbility_MontageBase.h"
#include "AlsAbilitySystemComponent.h"
#include "Animation/AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_MontageBase)

// --------------------------------------------------------------------------------------------------------------------------------------------------------
//
//	UAlsGameplayAbility_Montage
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------

void UAlsGameplayAbility_MontageBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	auto* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ThisClass::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ThisClass::OnNotifyEndReceived);
	}

	auto* const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent)
	{
		if (bWasCancelled && IsValid(GetCurrentMontage()))
		{
			AbilitySystemComponent->StopMontageIfCurrent(*GetCurrentMontage(), BlendOutDurationOnCancel);
			bStopCurrentMontageOnEndAbility = false;
		}

		// Remove any GameplayEffects that we applied

		for (auto& EffectHandle : AppliedEffects)
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle);
		}
		AppliedEffects.Reset();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAlsGameplayAbility_MontageBase::OnEndMontage_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bInterrupted);
	}
}

void UAlsGameplayAbility_MontageBase::GetGameplayEffectsWhileAnimating(TArray<const UGameplayEffect*>& OutEffects) const
{
	for (TSubclassOf<UGameplayEffect> EffectClass : GameplayEffectClassesWhileAnimating)
	{
		if (EffectClass)
		{
			OutEffects.Add(EffectClass->GetDefaultObject<UGameplayEffect>());
		}
	}
}

void UAlsGameplayAbility_MontageBase::PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FAlsPlayMontageParameter& Parameter,
												  const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	PlayMontage(ActivationInfo, Parameter.MontageToPlay, Parameter.PlayRate, Parameter.SectionName, Parameter.StartTime, Handle, ActorInfo);
}

void UAlsGameplayAbility_MontageBase::PlayMontage(UAnimMontage* Montage, float PlayRate, FName SectionName, float StartTime)
{
	PlayMontage(GetCurrentActivationInfo(), Montage, PlayRate, SectionName, StartTime, CurrentSpecHandle, GetCurrentActorInfo());
}

void UAlsGameplayAbility_MontageBase::PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo,
												  const FAlsPlayMontageParameter& Parameter)
{
	PlayMontage(ActivationInfo, Parameter, CurrentSpecHandle, ActorInfo);
}

void UAlsGameplayAbility_MontageBase::PlayMontage(const FAlsPlayMontageParameter& Parameter)
{
	PlayMontage(GetCurrentActivationInfo(), Parameter, CurrentSpecHandle, GetCurrentActorInfo());
}

void UAlsGameplayAbility_MontageBase::PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo,
												  UAnimMontage* Montage, float PlayRate, FName SectionName, float StartTime,
												  const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	auto* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();

	if (Montage != nullptr && AnimInstance != nullptr && AbilitySystemComponent)
	{
		AppliedEffects.Reset();

		// Apply GameplayEffects
		TArray<const UGameplayEffect*> Effects;
		GetGameplayEffectsWhileAnimating(Effects);
		if (Effects.Num() > 0)
		{
			for (const UGameplayEffect* Effect : Effects)
			{
				auto EffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(Effect, 1.f, MakeEffectContext(Handle, ActorInfo));
				if (EffectHandle.IsValid())
				{
					AppliedEffects.Add(EffectHandle);
				}
			}
		}

		CurrentMotangeDuration = AbilitySystemComponent->PlayMontage(this, ActivationInfo, Montage, PlayRate, SectionName, StartTime);

		if (CurrentMotangeDuration > 0.0f)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ThisClass::OnEndMontage);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

			if (auto* MontageInstance = AnimInstance->GetActiveInstanceForMontage(Montage))
			{
				// AnimInstance's OnPlayMontageNotifyBegin/End fire for all notify. Then stores Montage's InstanceID
				CurrentMontageInstanceId = MontageInstance->GetInstanceID();

				AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::OnNotifyBeginReceived);
				AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ThisClass::OnNotifyEndReceived);
			}
		}
	}
}

bool UAlsGameplayAbility_MontageBase::IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload) const
{
    return CurrentMontageInstanceId != INDEX_NONE && BPNPayload.MontageInstanceID == CurrentMontageInstanceId;
}

void UAlsGameplayAbility_MontageBase::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload)
{
    if (IsNotifyValid(NotifyName, BPNPayload))
    {
		if (NotifyName == FName(TEXT("EndAbility")))
		{
			EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		}

        float TriggerTime = BPNPayload.NotifyEvent ? BPNPayload.NotifyEvent->GetTriggerTime() : 0.f;
        float Duration = BPNPayload.NotifyEvent ? BPNPayload.NotifyEvent->GetDuration() : 0.f;
        OnNotifyBegin.Broadcast(NotifyName, TriggerTime, Duration);
    }
}

void UAlsGameplayAbility_MontageBase::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload)
{
    if (IsNotifyValid(NotifyName, BPNPayload))
    {
        float TriggerTime = BPNPayload.NotifyEvent ? BPNPayload.NotifyEvent->GetTriggerTime() : 0.f;
        float Duration = BPNPayload.NotifyEvent ? BPNPayload.NotifyEvent->GetDuration() : 0.f;
        OnNotifyEnd.Broadcast(NotifyName, TriggerTime, Duration);
    }
}
