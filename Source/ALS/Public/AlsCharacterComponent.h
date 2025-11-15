#pragma once

#include "Components/PawnComponent.h"
#include "AlsCharacterComponent.generated.h"

class AAlsCharacter;

UCLASS(Abstract)
class ALS_API UAlsCharacterComponent : public UPawnComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character Component|State", Transient)
	TWeakObjectPtr<AAlsCharacter> Character;

protected:
	virtual void OnRegister() override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterComponent")
	void OnPossessed(AController* NewController);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterComponent")
	void OnUnPossessed(AController* PreviousController);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|CharacterComponent")
	void OnRefresh(float DeltaTime);
};
