#include "Abilities/Tasks/AlsAbilityTask_OnTick.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilityTask_OnTick)

UAlsAbilityTask_Tick::UAlsAbilityTask_Tick(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bTickingTask = true;
}

UAlsAbilityTask_Tick *UAlsAbilityTask_Tick::New(UGameplayAbility* OwningAbility, const FName TaskInstanceName)
{
    return NewAbilityTask<UAlsAbilityTask_Tick>(OwningAbility, TaskInstanceName);
}

void UAlsAbilityTask_Tick::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnTick.Broadcast(DeltaTime);
    }
}
