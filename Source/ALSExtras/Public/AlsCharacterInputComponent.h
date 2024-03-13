#pragma once

#include "AlsCharacterComponent.h"
#include "AlsAbilityInputAction.h"
#include "Utility/AlsGameplayTags.h"
#include "AlsCharacterInputComponent.generated.h"

class UInputMappingContext;

UCLASS(AutoExpandCategories = ("AlsCharacterInput|Settings"))
class ALSEXTRAS_API UAlsCharacterInputComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings")
	TArray<FAlsAbilityInputAction> AbilityInputActions;

protected:
	virtual void OnRegister() override;

	virtual void OnControllerChanged_Implementation(AController* PreviousController, AController* NewController) override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterComponent")
	void OnSetupPlayerInputComponent(UInputComponent* Input);
};
