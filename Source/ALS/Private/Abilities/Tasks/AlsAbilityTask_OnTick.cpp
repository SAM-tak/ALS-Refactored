#include "Abilities/Tasks/AlsAbilityTask_OnTick.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilityTask_OnTick)

UAlsAbilityTask_OnTick::UAlsAbilityTask_OnTick(const FObjectInitializer &ObjectInitializer)
{
    bTickingTask = true;
}

UAlsAbilityTask_OnTick *UAlsAbilityTask_OnTick::AbilityTaskOnTick(UGameplayAbility *OwningAbility, const FName TaskInstanceName)
{
    UAlsAbilityTask_OnTick* MyObj = NewAbilityTask<UAlsAbilityTask_OnTick>(OwningAbility, TaskInstanceName);
    return MyObj;
}

void UAlsAbilityTask_OnTick::Activate()
{
    Super::Activate();
}

void UAlsAbilityTask_OnTick::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnTick.Broadcast(DeltaTime);
    }
}
