#pragma once

#include "Components/PawnComponent.h"
#include "AlsCharacterComponent.generated.h"

class AAlsCharacter;

UCLASS(Abstract)
class ALS_API UAlsCharacterComponent : public UPawnComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Transient, Category = "State|Als Character Component")
	TWeakObjectPtr<AAlsCharacter> Character;

protected:
	virtual void OnRegister() override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterComponent")
	void OnControllerChanged(AController *PreviousController, AController* NewController);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterComponent")
	void OnSetupPlayerInputComponent(UInputComponent* Input);
};
