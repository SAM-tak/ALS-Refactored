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

UAlsGameplayAbility_MontageBase::UAlsGameplayAbility_MontageBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

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
		if (bWasCancelled)
		{
			AbilitySystemComponent->CurrentMontageStop(BlendOutDurationOnCancel);
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
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
				   ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, bInterrupted);
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

void UAlsGameplayAbility_MontageBase::StopMontage(float OverrideBlendOutTime)
{
	auto* const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CurrentMontageStop(OverrideBlendOutTime);
	}
}

void UAlsGameplayAbility_MontageBase::PlayMontage(const FGameplayAbilityActivationInfo ActivationInfo, const FAlsPlayMontageParameter& Parameter,
												  const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	PlayMontage(GetCurrentActivationInfo(), Parameter.MontageToPlay, Parameter.PlayRate, Parameter.SectionName, Parameter.StartTime,
				CurrentSpecHandle, GetCurrentActorInfo());
}

void UAlsGameplayAbility_MontageBase::PlayMontage(UAnimMontage* Montage, float PlayRate, FName SectionName, float StartTime)
{
	PlayMontage(GetCurrentActivationInfo(), Montage, PlayRate, SectionName, StartTime, CurrentSpecHandle, GetCurrentActorInfo());
}

void UAlsGameplayAbility_MontageBase::PlayMontage(
	const FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float PlayRate, FName SectionName, float StartTime,
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
				CurrentMontageInstanceID = MontageInstance->GetInstanceID();

				AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::OnNotifyBeginReceived);
				AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ThisClass::OnNotifyEndReceived);
			}
		}
	}
}

bool UAlsGameplayAbility_MontageBase::IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload) const
{
    return CurrentMontageInstanceID != INDEX_NONE && BPNPayload.MontageInstanceID == CurrentMontageInstanceID;
}

void UAlsGameplayAbility_MontageBase::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload)
{
    if (IsNotifyValid(NotifyName, BPNPayload))
    {
		if (NotifyName == FName(TEXT("EndAbility")))
		{
			EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
					   ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, false);
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