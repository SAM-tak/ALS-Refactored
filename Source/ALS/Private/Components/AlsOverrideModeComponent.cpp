#include "Components/AlsOverrideModeComponent.h"

#include "AlsCharacter.h"
#include "AlsLinkedAnimationInstance.h"
#include "CharacterTasks/AlsOverrideTask.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverrideModeComponent)

void UAlsOverrideModeComponent::BeginPlay()
{
	Super::BeginPlay();

	InstancedOverrideTasks.Reset();
}

void UAlsOverrideModeComponent::ChangeOverrideTaskIfNeeded(const FGameplayTag& OverrideMode)
{
	if (IsValid(CurrentOverrideTask))
	{
		auto* Key{OverrideClassMap.FindKey(CurrentOverrideTask->GetClass())};
		if (Key && (*Key) == OverrideMode)
		{
			return;
		}
		else
		{
			CurrentOverrideTask->End();
			CurrentOverrideTask = nullptr;
		}
	}

	if (OverrideClassMap.Contains(OverrideMode))
	{
		auto& TaskClass{OverrideClassMap[OverrideMode]};
		if (InstancedOverrideTasks.Contains(TaskClass))
		{
			CurrentOverrideTask = InstancedOverrideTasks[TaskClass].Get();
		}
		else
		{
			CurrentOverrideTask = NewObject<UAlsOverrideTask>(Character.Get(), TaskClass);
			CurrentOverrideTask->OnRegister();
			InstancedOverrideTasks.Add(TaskClass, CurrentOverrideTask);
		}
		CurrentOverrideTask->Begin();
	}
}

void UAlsOverrideModeComponent::OnRefresh_Implementation(float DeltaTime)
{
	Super::OnRefresh_Implementation(DeltaTime);
	if (IsValid(CurrentOverrideTask))
	{
		CurrentOverrideTask->Refresh(DeltaTime);
	}
}

void UAlsOverrideModeComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	Super::OnControllerChanged_Implementation(PreviousController, NewController);
	if (IsValid(CurrentOverrideTask))
	{
		CurrentOverrideTask->OnControllerChanged(PreviousController, NewController);
	}
}
