// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AlsCharacterTask.h"
#include "AlsLocalMontageTask.generated.h"

/**
 *
 */
UCLASS(Abstract)
class ALS_API UAlsLocalMontageTask : public UAlsCharacterTask
{
	GENERATED_BODY()

	friend class UAlsLocalMontageComponent;
	friend class UAlsAbilityTask_PlayLocalMontage;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LocalMontageTask|Settings", Transient)
	uint8 bStopCurrentMontageOnEnd : 1{true};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LocalMontageTask|State", Transient)
	TWeakObjectPtr<class UAlsLocalMontageComponent> Component;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LocalMontageTask|State", Transient)
	TWeakObjectPtr<class UAnimMontage> CurrentMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LocalMontageTask|State", Transient)
	TWeakObjectPtr<class UAlsAbilityTask_PlayLocalMontage> CurrentAbilityTask;

	UFUNCTION(BlueprintCallable, Category = "ALS|LocalMontageTask")
	virtual bool Play(const struct FAlsPlayMontageParameter& Parameter);

	UFUNCTION(BlueprintCallable, Category = "ALS|LocalMontageTask")
	virtual void Stop(float OverrideBlendOutTime = -1.0f);

	UFUNCTION()
	virtual void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	virtual void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|LocalMontageTask", DisplayName = "On Notify Begin Received", Meta = (ScriptName = "OnNotifyBeginReceived"))
	void K2_OnNotifyBegin(FName NotifyName);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|LocalMontageTask", DisplayName = "On Notify End Received", Meta = (ScriptName = "OnNotifyEndReceived"))
	void K2_OnNotifyEnd(FName NotifyName);

	void OnEndMontage(class UAnimMontage* Montage, bool bInterrupted);

	virtual void OnEnd(bool bWasCancelled) override;

private:
	int32 MontageInstanceID{INDEX_NONE};

	bool IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const;
};
