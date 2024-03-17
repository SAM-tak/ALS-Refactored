#pragma once

#include "AlsCharacterComponent.h"
#include "AlsOverlayModeComponent.generated.h"

class UAlsOverlayTask;

UCLASS(Abstract, AutoExpandCategories = ("AlsOverlayModeComponent|Settings"))
class ALS_API UAlsOverlayModeComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsOverlayModeComponent|Settings", Meta = (DisplayThumbnail = false))
	TMap<FGameplayTag, TSubclassOf<UAlsOverlayTask>> OverlayClassMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsOverlayModeComponent|State", Transient)
	TObjectPtr<UAlsOverlayTask> CurrentOverlayTask;

	TMap<TSubclassOf<UAlsOverlayTask>, TObjectPtr<UAlsOverlayTask>> InstancedOverlayTasks;

protected:
	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	virtual void OnRefresh_Implementation(float DeltaTime) override;

	virtual void OnControllerChanged_Implementation(AController* PreviousController, AController* NewController) override;

	void ChangeOverlayTask(const FGameplayTag& OverlayMode);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|OverlayModeComponent")
	void OnChangeOverlayMode(const FGameplayTag& PreviousOverlayMode);
};
