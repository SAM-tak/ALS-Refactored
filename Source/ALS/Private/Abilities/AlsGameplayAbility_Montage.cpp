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

void UAlsGameplayAbility_Montage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData *TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance();

	if (MontageToPlay != nullptr && AnimInstance != nullptr && AnimInstance->GetActiveMontageInstance() == nullptr)
	{
		AppliedEffects.Reset();

		// Apply GameplayEffects
		TArray<const UGameplayEffect *> Effects;
		GetGameplayEffectsWhileAnimating(Effects);
		if (Effects.Num() > 0)
		{
			UAbilitySystemComponent *const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
			for (const UGameplayEffect *Effect : Effects)
			{
				FActiveGameplayEffectHandle EffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(Effect, 1.f, MakeEffectContext(Handle, ActorInfo));
				if (EffectHandle.IsValid())
				{
					AppliedEffects.Add(EffectHandle);
				}
			}
		}

		float const Duration = AnimInstance->Montage_Play(MontageToPlay, PlayRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UAlsGameplayAbility_Montage::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate);

		if (SectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAlsGameplayAbility_Montage::OnMontageEnded_Implementation(UAnimMontage *Montage, bool bInterrupted)
{
	// Remove any GameplayEffects that we applied
	UAbilitySystemComponent *const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent)
	{
		for (FActiveGameplayEffectHandle Handle : AppliedEffects)
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}

		if (EndsAbilityOnMontageEnded)
		{
			EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
				ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, bInterrupted);
		}
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
