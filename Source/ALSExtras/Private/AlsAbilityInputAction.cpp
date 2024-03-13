#include "AlsAbilityInputAction.h"
#include "AlsAbilitySystemComponent.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilityInputAction)

void FAlsAbilityInputAction::BindAction(UAlsAbilitySystemComponent* AbilitySystemComponent, UEnhancedInputComponent* InputComponent) const
{
	AbilitySystemComponent->BindAbilityActivationInput(InputComponent, InputAction, TriggerEvent, InputTag);
}
