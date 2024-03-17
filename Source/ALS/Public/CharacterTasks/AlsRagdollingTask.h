// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterTasks/AlsOverrideTask.h"
#include "AlsRagdollingTask.generated.h"

class UAlsLinkedAnimationInstance;

/**
 * Ragdolling
 */
UCLASS(Abstract)
class ALS_API UAlsRagdollingTask : public UAlsOverrideTask
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bOnGroundedAndAgedFired : 1{false};

	UFUNCTION(BlueprintPure, Category = "ALS|CharacterTask|Ragdolling")
	bool IsGroundedAndAged() const;

public:
	virtual void Refresh(float DeltaTime) override;

protected:
	virtual void OnEnd(bool bWasCancelled) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|CharacterTask|Ragdolling", DisplayName = "On Grounded And Aged", Meta = (ScriptName = "OnGroundedAndAged"))
	void K2_OnGroundedAndAged();
};
