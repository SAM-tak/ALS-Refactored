#include "Components/AlsOverrideModeComponent.h"

#include "AlsCharacter.h"
#include "AlsLinkedAnimationInstance.h"
#include "CharacterTasks/AlsOverrideTask.h"
#include "CharacterTasks/AlsRagdollingTask.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverrideModeComponent)

void UAlsOverrideModeComponent::BeginPlay()
{
	Super::BeginPlay();

	InstancedOverrideTasks.Reset();
	OverrideTagsMask.Reset();
	for (auto& KeyValue : OverrideClassMap)
	{
		OverrideTagsMask.AddTag(KeyValue.Key);
	}
}

void UAlsOverrideModeComponent::EndCurrentRagdollingTask()
{
	if (CurrentOverrideTask.IsValid() && CurrentOverrideTask->IsA(UAlsRagdollingTask::StaticClass()))
	{
		CurrentOverrideTask->End();
		CurrentOverrideTask.Reset();
		CurrentOverrideTag = FGameplayTag::EmptyTag;
	}
}

void UAlsOverrideModeComponent::ChangeOverrideTaskIfNeeded(const FGameplayTag& OverrideMode)
{
	if (CurrentOverrideTask.IsValid())
	{
		if (CurrentOverrideTag == OverrideMode)
		{
			return;
		}
		else
		{
			CurrentOverrideTask->End();
			CurrentOverrideTask.Reset();
			CurrentOverrideTag = FGameplayTag::EmptyTag;
		}
	}

	if (OverrideClassMap.Contains(OverrideMode))
	{
		if (InstancedOverrideTasks.Contains(OverrideMode))
		{
			CurrentOverrideTask = InstancedOverrideTasks[OverrideMode];
		}
		else
		{
			auto* NewTask{NewObject<UAlsOverrideTask>(Character.Get(), OverrideClassMap[OverrideMode])};
			InstancedOverrideTasks.Add(OverrideMode, NewTask);
			CurrentOverrideTask = NewTask;
			CurrentOverrideTask->OnRegister();
		}
		CurrentOverrideTag = OverrideMode;
		CurrentOverrideTask->Begin();
	}
}

void UAlsOverrideModeComponent::OnRefresh_Implementation(float DeltaTime)
{
	Super::OnRefresh_Implementation(DeltaTime);

	FGameplayTagContainer Container;
	Character->GetOwnedGameplayTags(Container);
	auto CurrentOverrideTags{Container.Filter(OverrideTagsMask)};

	ChangeOverrideTaskIfNeeded(CurrentOverrideTags.First());

	if (CurrentOverrideTask.IsValid())
	{
		CurrentOverrideTask->Refresh(DeltaTime);
	}
}

void UAlsOverrideModeComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	Super::OnControllerChanged_Implementation(PreviousController, NewController);
	if (CurrentOverrideTask.IsValid())
	{
		CurrentOverrideTask->OnControllerChanged(PreviousController, NewController);
	}
}
