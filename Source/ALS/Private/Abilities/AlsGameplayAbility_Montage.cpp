// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/AlsGameplayAbility_Montage.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Montage)

// --------------------------------------------------------------------------------------------------------------------------------------------------------
//
//	UAlsGameplayAbility_Montage
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------

UAlsGameplayAbility_Montage::UAlsGameplayAbility_Montage(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UAlsGameplayAbility_Montage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
												  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData *TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	PlayMontage(MontageToPlay, Handle, ActorInfo, ActivationInfo);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if(CurrentMotangeDuration <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, false);
	}
}

void UAlsGameplayAbility_Montage::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

void UAlsGameplayAbility_Montage::OnEndMontage_Implementation(UAnimMontage *Montage, bool bInterrupted)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
				   ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, bInterrupted);
	}
}

void UAlsGameplayAbility_Montage::GetGameplayEffectsWhileAnimating(TArray<const UGameplayEffect *> &OutEffects) const
{
	for (TSubclassOf<UGameplayEffect> EffectClass : GameplayEffectClassesWhileAnimating)
	{
		if (EffectClass)
		{
			OutEffects.Add(EffectClass->GetDefaultObject<UGameplayEffect>());
		}
	}
}

void UAlsGameplayAbility_Montage::StopMontage(float OverrideBlendOutTime)
{
	auto* const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CurrentMontageStop(OverrideBlendOutTime);
	}
}

void UAlsGameplayAbility_Montage::PlayMontage(UAnimMontage* Montage)
{
	PlayMontage(Montage, CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo());
}

void UAlsGameplayAbility_Montage::PlayMontage(UAnimMontage* Montage, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											  const FGameplayAbilityActivationInfo ActivationInfo)
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

bool UAlsGameplayAbility_Montage::IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload) const
{
    return CurrentMontageInstanceID != INDEX_NONE && BPNPayload.MontageInstanceID == CurrentMontageInstanceID;
}

void UAlsGameplayAbility_Montage::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload)
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

void UAlsGameplayAbility_Montage::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BPNPayload)
{
    if (IsNotifyValid(NotifyName, BPNPayload))
    {
        float TriggerTime = BPNPayload.NotifyEvent ? BPNPayload.NotifyEvent->GetTriggerTime() : 0.f;
        float Duration = BPNPayload.NotifyEvent ? BPNPayload.NotifyEvent->GetDuration() : 0.f;
        OnNotifyEnd.Broadcast(NotifyName, TriggerTime, Duration);
    }
}
