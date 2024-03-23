#include "AlsAbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "Abilities/AlsGameplayAbility.h"
#include "Animation/AnimMontage.h"
#include "RootMotionSources/AlsRootMotionSource_Mantling.h"
#include "Abilities/Actions/AlsGameplayAbility_Mantling.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilitySystemComponent)

UAlsAbilitySystemComponent::UAlsAbilitySystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicated(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UAlsAbilitySystemComponent::OnRegister()
{
	Super::OnRegister();
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	if (IsValid(Character))
	{
		Character->OnRefresh.AddUObject(this, &ThisClass::OnRefresh);
		Character->OnContollerChanged.AddUObject(this, &ThisClass::OnControllerChanged);
	}
}

void UAlsAbilitySystemComponent::BindAbilityActivationInput(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action, ETriggerEvent TriggerEvent,
														    const FGameplayTag& InputTag)
{
	auto Handle{EnhancedInputComponent->BindAction(Action, TriggerEvent, this, &ThisClass::ActivateOnInputAction, InputTag).GetHandle()};
	if (!BindingHandles.Contains(InputTag))
	{
		BindingHandles.Add(InputTag);
	}
	BindingHandles[InputTag].AddUnique(Handle);
}

void UAlsAbilitySystemComponent::UnbindAbilityInputs(UEnhancedInputComponent* EnhancedInputComponent, const FGameplayTag& InputTag)
{
	if (BindingHandles.Contains(InputTag))
	{
		for(auto Handle : BindingHandles[InputTag])
		{
			EnhancedInputComponent->RemoveActionBindingForHandle(Handle);
		}
		BindingHandles.Remove(InputTag);
	}
}

void UAlsAbilitySystemComponent::ActivateOnInputAction(FGameplayTag InputTag)
{
	TryActivateAbilitiesBySingleTag(InputTag);
}

void UAlsAbilitySystemComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	RefreshAbilityActorInfo();

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.IsActive())
		{
			auto* AlsAbility{Cast<UAlsGameplayAbility>(Spec.Ability)};
			if (IsValid(AlsAbility))
			{
				AlsAbility->OnControllerChanged(PreviousController, NewController);
			}
		}
	}
}

void UAlsAbilitySystemComponent::OnRefresh_Implementation(float DeltaTime)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};

	if (Character->GetLocomotionMode() == AlsLocomotionModeTags::InAir && Character->IsLocallyControlled())
	{
		TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::Mantling);
	}
}

void UAlsAbilitySystemComponent::SetGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate)
{
	SetLooseGameplayTagCount(Tag, 1);
	if (bShouldReplicate)
	{
		SetReplicatedLooseGameplayTagCount(Tag, 1);
	}
}

void UAlsAbilitySystemComponent::ResetGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate)
{
	SetLooseGameplayTagCount(Tag, 0);
	if (bShouldReplicate)
	{
		SetReplicatedLooseGameplayTagCount(Tag, 0);
	}
}

void UAlsAbilitySystemComponent::AddGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate)
{
	AddLooseGameplayTag(Tag);
	if (bShouldReplicate)
	{
		AddReplicatedLooseGameplayTag(Tag);
	}
}

void UAlsAbilitySystemComponent::SubtractGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate)
{
	RemoveLooseGameplayTag(Tag);
	if (bShouldReplicate)
	{
		RemoveReplicatedLooseGameplayTag(Tag);
	}
}

void UAlsAbilitySystemComponent::K2_SetGameplayTag(FGameplayTag Tag, bool bShouldReplicate)
{
	SetGameplayTag(Tag, bShouldReplicate);
}

void UAlsAbilitySystemComponent::K2_ResetGameplayTag(FGameplayTag Tag, bool bShouldReplicate)
{
	ResetGameplayTag(Tag, bShouldReplicate);
}

void UAlsAbilitySystemComponent::K2_AddGameplayTag(FGameplayTag Tag, bool bShouldReplicate)
{
	AddGameplayTag(Tag, bShouldReplicate);
}

void UAlsAbilitySystemComponent::K2_SubtractGameplayTag(FGameplayTag Tag, bool bShouldReplicate)
{
	SubtractGameplayTag(Tag, bShouldReplicate);
}
