#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AlsAbilityTask_Tick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAlsAbilityTask_OnTickDelegate, float, DeltaTime);

/**
 * Task for abilities that supply tick and its' delta time.
 */
UCLASS()
class ALS_API UAlsAbilityTask_Tick : public UAbilityTask
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FAlsAbilityTask_OnTickDelegate OnTick;

    UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Tasks", meta = (DisplayName = "New Tick Ability Task", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAlsAbilityTask_Tick* New(UGameplayAbility* OwningAbility, const FName TaskInstanceName);

    virtual void TickTask(float DeltaTime) override;
};
