#pragma once

#include "AbilitySystemComponent.h"
#include "InputTriggers.h"
#include "AlsAbilitySystemComponent.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class AAlsCharacter;

/**
 * AbilitySystemComponent for ALS Refactored
 */
UCLASS()
class ALS_API UAlsAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void OnRegister() override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|AbilitySystem")
	void OnControllerChanged(AController* PreviousController, AController* NewController);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|AbilitySystem")
	void OnRefresh(float DeltaTime);

	// Utility

public:
	inline void CancelAbilitiesBySingleTag(const FGameplayTag& Tag)
	{
		FGameplayTagContainer TagContainer{Tag};
		CancelAbilities(&TagContainer);
	}

	inline bool TryActivateAbilitiesBySingleTag(const FGameplayTag& Tag, bool bAllowRemoteActivation = true)
	{
		return TryActivateAbilitiesByTag(FGameplayTagContainer{Tag}, bAllowRemoteActivation);
	}

	// Input binding

public:
	void BindAbilityActivationInput(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action, ETriggerEvent TriggerEvent, const FGameplayTag& InputTag);

	void UnbindAbilityInputs(UEnhancedInputComponent* EnhancedInputComponent, const FGameplayTag& InputTag);

private:
	TMap<FGameplayTag, TArray<uint32>> BindingHandles;

	void ActivateOnInputAction(FGameplayTag InputTag);
};
