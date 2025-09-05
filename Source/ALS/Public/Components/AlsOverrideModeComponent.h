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
	FGameplayTag CurrentOverrideTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsOverrideModeComponent|State", Transient)
	TWeakObjectPtr<UAlsOverrideTask> CurrentOverrideTask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsOverrideModeComponent|State", Transient)
	FGameplayTagContainer OverrideTagsMask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsOverrideModeComponent|State", Transient)
	TMap<FGameplayTag, TObjectPtr<UAlsOverrideTask>> InstancedOverrideTasks;

public:
	UFUNCTION(BlueprintCallable)
	void EndCurrentRagdollingTask();

protected:
	virtual void BeginPlay() override;

	virtual void OnRefresh_Implementation(float DeltaTime) override;

	virtual void OnPossessed_Implementation(AController* NewController) override;

	virtual void OnUnPossessed_Implementation(AController* PreviousController) override;

	void ChangeOverrideTaskIfNeeded(const FGameplayTag& Tag);
};
