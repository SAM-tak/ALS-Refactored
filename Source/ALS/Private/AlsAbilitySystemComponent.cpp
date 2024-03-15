#include "AlsAbilitySystemComponent.h"
#include "EnhancedInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilitySystemComponent)

UAlsAbilitySystemComponent::UAlsAbilitySystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicated(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
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
