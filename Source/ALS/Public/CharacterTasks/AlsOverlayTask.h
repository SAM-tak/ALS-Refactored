// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AlsCharacterTask.h"
#include "AlsOverlayTask.generated.h"

class UAlsOverlayAnimInstance;

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsOverlayTask : public UAlsCharacterTask
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UAlsOverlayAnimInstance> OverlayAnimInstance;

public:
	virtual void Begin() override;

protected:
	virtual void OnEnd(bool bWasCancelled) override;
};
