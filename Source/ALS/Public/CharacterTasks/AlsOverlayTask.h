// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AlsCharacterTask.h"
#include "AlsOverlayTask.generated.h"

class UAlsOverlayAnimInstance;
class UAlsOverlayModeComponent;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsOverlayTask : public UAlsCharacterTask
{
	GENERATED_BODY()

	friend UAlsOverlayModeComponent;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UAlsOverlayAnimInstance> OverlayAnimClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsOverlayModeComponent> Component;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsOverlayAnimInstance> OverlayAnimInstance;

public:
	virtual void Begin() override;

	virtual void Refresh(float DeltaTime) override;

protected:
	virtual void OnFinished() override;
};
