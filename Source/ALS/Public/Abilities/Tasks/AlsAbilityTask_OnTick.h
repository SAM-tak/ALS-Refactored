#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AlsAbilityTask_OnTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickTaskDelegate, float, DeltaTime);

/**
 * Task for abilities that supply tick and its' delta time.
 */
UCLASS()
class ALS_API UAlsAbilityTask_OnTick : public UAbilityTask
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintAssignable)
    FOnTickTaskDelegate OnTick;

public:
    UAlsAbilityTask_OnTick(const FObjectInitializer &ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAlsAbilityTask_OnTick *AbilityTaskOnTick(UGameplayAbility *OwningAbility, const FName TaskInstanceName);

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
};
