#pragma once

#include "AlsCharacterComponent.h"
#include "Abilities/AlsAbilityInputAction.h"
#include "AlsCharacterInputComponent.generated.h"

class UInputMappingContext;

UCLASS(Abstract, AutoExpandCategories = ("AlsCharacterInput|Settings"))
class ALS_API UAlsCharacterInputComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings")
	TArray<FAlsAbilityInputAction> AbilityInputActions;

protected:
	virtual void OnRegister() override;

	virtual void OnPossessed_Implementation(AController* NewController) override;

	virtual void OnUnPossessed_Implementation(AController* PreviousController) override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterInput")
	void OnSetupPlayerInputComponent(UInputComponent* Input);
};
