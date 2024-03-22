// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AlsCharacterTask.h"
#include "AlsOverrideTask.generated.h"

class UAlsCharacterTaskAnimInstance;
class UAlsOverrideModeComponent;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsOverrideTask : public UAlsCharacterTask
{
	GENERATED_BODY()

	friend UAlsOverrideModeComponent;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UAlsCharacterTaskAnimInstance> OverrideAnimClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsOverrideModeComponent> Component;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsCharacterTaskAnimInstance> OverrideAnimInstance;

public:
	virtual void Begin() override;

	virtual void Refresh(float DeltaTime) override;

protected:
	virtual void OnFinished() override;
};
