#pragma once

#include "AlsCharacterComponent.h"
#include "AlsOverrideModeComponent.generated.h"

class UAlsOverrideTask;

UCLASS(Abstract, AutoExpandCategories = ("AlsOverrideModeComponent|Settings"))
class ALS_API UAlsOverrideModeComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsOverrideModeComponent|Settings", Meta = (DisplayThumbnail = false))
	TMap<FGameplayTag, TSubclassOf<UAlsOverrideTask>> OverrideClassMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsOverrideModeComponent|State", Transient)
	TObjectPtr<UAlsOverrideTask> CurrentOverrideTask;

	TMap<TSubclassOf<UAlsOverrideTask>, TObjectPtr<UAlsOverrideTask>> InstancedOverrideTasks;

protected:
	virtual void BeginPlay() override;

	virtual void OnRefresh_Implementation(float DeltaTime) override;

	virtual void OnControllerChanged_Implementation(AController* PreviousController, AController* NewController) override;

	void ChangeOverrideTaskIfNeeded(const FGameplayTag& Tag);
};
