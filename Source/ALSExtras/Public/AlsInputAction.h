#pragma once

#include "GameplayTagContainer.h"
#include "AlsInputAction.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UAlsAbilitySystemComponent;

// Behavior Type
UENUM()
enum class EAlsInputActionType : uint8
{
	// Ignore
	None,

	// Activate Ability
	Activate,

	// Cancel Ability
	Cancel,

	// Activates if inactive, cancels if active
	Toggle,
};

/**
 * FAlsInputAction
 *
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct ALSEXTRAS_API FAlsInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (DisplayThumbnail = false))
	TObjectPtr<const UInputAction> InputAction{nullptr};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Behavior"))
	EAlsInputActionType OnTrigger{EAlsInputActionType::Activate};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Behavior"))
	EAlsInputActionType OnRelease{EAlsInputActionType::None};

	void BindAction(UAlsAbilitySystemComponent* AbilitySystemComponent, UEnhancedInputComponent* InputComponent);
};
