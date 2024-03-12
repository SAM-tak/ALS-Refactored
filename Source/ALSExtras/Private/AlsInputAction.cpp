#include "AlsInputAction.h"
#include "AlsAbilitySystemComponent.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsInputAction)

void FAlsInputAction::BindAction(UAlsAbilitySystemComponent* AbilitySystemComponent, UEnhancedInputComponent* InputComponent)
{
	ALS_ENSURE(OnTrigger != EAlsInputActionType::None || OnRelease != EAlsInputActionType::None);

	switch (OnTrigger)
	{
	case EAlsInputActionType::Activate:
		AbilitySystemComponent->BindActivateInput(InputComponent, InputAction, ETriggerEvent::Triggered, InputTag);
		break;
	case EAlsInputActionType::Toggle:
		AbilitySystemComponent->BindToggleInput(InputComponent, InputAction, ETriggerEvent::Triggered, InputTag);
		break;
	case EAlsInputActionType::Cancel:
		AbilitySystemComponent->BindCancelInput(InputComponent, InputAction, ETriggerEvent::Triggered, InputTag);
		break;
	}
	switch (OnRelease)
	{
	case EAlsInputActionType::Activate:
		AbilitySystemComponent->BindActivateInput(InputComponent, InputAction, ETriggerEvent::Completed, InputTag);
		break;
	case EAlsInputActionType::Toggle:
		AbilitySystemComponent->BindToggleInput(InputComponent, InputAction, ETriggerEvent::Completed, InputTag);
		break;
	case EAlsInputActionType::Cancel:
		AbilitySystemComponent->BindCancelInput(InputComponent, InputAction, ETriggerEvent::Completed, InputTag);
		break;
	}
}
