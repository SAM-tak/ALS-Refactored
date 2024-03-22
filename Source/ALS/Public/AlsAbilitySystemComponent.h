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

	void SetGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate = false);

	void ResetGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate = false);

	void AddGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate = false);

	void SubtractGameplayTag(const FGameplayTag& Tag, bool bShouldReplicate = false);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Set GameplayTag", ScriptName = "SetGameplayTag"))
	void K2_SetGameplayTag(FGameplayTag Tag, bool bShouldReplicate);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Reset GameplayTag", ScriptName = "ResetGameplayTag"))
	void K2_ResetGameplayTag(FGameplayTag Tag, bool bShouldReplicate);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Add GameplayTag", ScriptName = "AddGameplayTag"))
	void K2_AddGameplayTag(FGameplayTag Tag, bool bShouldReplicate);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability", Meta = (DisplayName = "Subtract GameplayTag", ScriptName = "SubtractGameplayTag"))
	void K2_SubtractGameplayTag(FGameplayTag Tag, bool bShouldReplicate);

	// Input binding

public:
	void BindAbilityActivationInput(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action, ETriggerEvent TriggerEvent, const FGameplayTag& InputTag);

	void UnbindAbilityInputs(UEnhancedInputComponent* EnhancedInputComponent, const FGameplayTag& InputTag);

private:
	TMap<FGameplayTag, TArray<uint32>> BindingHandles;

	void ActivateOnInputAction(FGameplayTag InputTag);
};
