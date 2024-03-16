#pragma once

#include "AlsCharacterComponent.h"
#include "AlsOverlayModeComponent.generated.h"

class UAlsOverlayAnimInstance;

UCLASS(Abstract, AutoExpandCategories = ("AlsOverlayModeComponent|Settings"))
class ALS_API UAlsOverlayModeComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsOverlayModeComponent|Settings", Meta = (DisplayThumbnail = false))
	TMap<FGameplayTag, TSubclassOf<UAlsOverlayAnimInstance>> OverlayAnimLayerMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsOverlayModeComponent|State", Transient)
	FGameplayTag CurrentOverlayMode;

protected:
	virtual void OnRefresh_Implementation(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|OverlayModeComponent")
	void OnChangeOverlayMode(const FGameplayTag& NewOverlayMode);

	UFUNCTION(BlueprintCallable, Category = "ALS|OverlayModeComponent")
	void LinkAnimLayer(const FGameplayTag& Tag);
};
