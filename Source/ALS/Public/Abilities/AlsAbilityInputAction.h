#pragma once

#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "AlsAbilityInputAction.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UAlsAbilitySystemComponent;

/**
 * FAlsAbilityInputAction
 *
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct ALS_API FAlsAbilityInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (DisplayThumbnail = false))
	TObjectPtr<const UInputAction> InputAction{nullptr};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Trigger"))
	ETriggerEvent TriggerEvent{ETriggerEvent::Triggered};

	void BindAction(UAlsAbilitySystemComponent* AbilitySystemComponent, UEnhancedInputComponent* InputComponent) const;
};
