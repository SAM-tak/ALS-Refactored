#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "PlayMontageCallbackProxy.h"
#include "AlsAbilityTask_PlayLocalMontage.generated.h"

/**
 * Task for abilities that supply tick and its' delta time.
 */
UCLASS()
class ALS_API UAlsAbilityTask_PlayLocalMontage : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag LocalMontageTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<class UAlsLocalMontageTask> LocalMontageTask;

	UPROPERTY(BlueprintAssignable)
	FOnMontagePlayDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnMontagePlayDelegate OnInterrupted;

	UPROPERTY(BlueprintAssignable)
	FOnMontagePlayDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable)
	FOnMontagePlayDelegate OnNotifyEnd;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Tasks", meta = (DisplayName = "PlayLocalMontage", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAlsAbilityTask_PlayLocalMontage* CreatePlayLocalMontage(UGameplayAbility* OwningAbility, FName TaskInstanceName, FGameplayTag _LocalMontageTag);

	virtual void Activate() override;

protected:
	virtual void OnDestroy(bool AbilityEnded) override;
};
