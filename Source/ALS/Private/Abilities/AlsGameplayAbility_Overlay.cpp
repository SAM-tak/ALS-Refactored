// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/AlsGameplayAbility_Overlay.h"
#include "AlsCharacter.h"
#include "LinkedAnimLayers/AlsOverlayAnimInstance.h"
#include "AlsAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Overlay)

UAlsGameplayAbility_Overlay::UAlsGameplayAbility_Overlay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	CancelAbilitiesWithTag.AddTag(AlsOverlayModeTags::Root);
}

void UAlsGameplayAbility_Overlay::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	auto* Character{GetAlsCharacterFromActorInfo()};

	UE_LOG(LogTemp, Log, TEXT("UAlsGameplayAbility_Overlay::ActivateAbility %s %d"), *Character->GetName(), bHasBlueprintActivate);

	Character->GetMesh()->LinkAnimClassLayers(OwnAnimLayersClass);

	auto PreviousOverlayMode{Character->GetOverlayMode()};

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
