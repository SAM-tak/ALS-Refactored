// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/AlsAbilitySet.h"

#include "AlsAbilitySystemComponent.h"
#include "Abilities/AlsGameplayAbility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilitySet)

void FAlsAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAlsAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FAlsAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

void FAlsAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* AlsASC)
{
	check(AlsASC);

	if (!AlsASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (const auto& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AlsASC->ClearAbility(Handle);
		}
	}

	for (const auto& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AlsASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (const auto& Set : GrantedAttributeSets)
	{
		AlsASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UAlsAbilitySet::UAlsAbilitySet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAlsAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* AlsASC, FAlsAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(AlsASC);

	if (!AlsASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex{0}; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const auto& AbilityToGrant{GrantedGameplayAbilities[AbilityIndex]};

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogAls, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		auto* AbilityCDO{AbilityToGrant.Ability->GetDefaultObject<UAlsGameplayAbility>()};

		FGameplayAbilitySpec AbilitySpec{AbilityCDO, AbilityToGrant.AbilityLevel};
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

		const auto AbilitySpecHandle{AlsASC->GiveAbility(AbilitySpec)};

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Grant the gameplay effects.
	for (int32 EffectIndex{0}; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const auto& EffectToGrant{GrantedGameplayEffects[EffectIndex]};

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogAls, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

		const auto* GameplayEffect{EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>()};
		const auto GameplayEffectHandle{AlsASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, AlsASC->MakeEffectContext())};

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	// Grant the attribute sets.
	for (int32 SetIndex{0}; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const auto& SetToGrant{GrantedAttributes[SetIndex]};

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogAls, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		auto* NewSet{NewObject<UAttributeSet>(AlsASC->GetOwner(), SetToGrant.AttributeSet)};
		AlsASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}
