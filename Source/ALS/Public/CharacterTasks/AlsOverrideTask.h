// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AlsCharacterTask.h"
#include "AlsOverrideTask.generated.h"

class UAlsLinkedAnimationInstance;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsOverrideTask : public UAlsCharacterTask
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UAlsLinkedAnimationInstance> OverrideAnimClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsLinkedAnimationInstance> OverrideAnimInstance;

public:
	virtual void Begin() override;

protected:
	virtual void OnEnd(bool bWasCancelled) override;
};
