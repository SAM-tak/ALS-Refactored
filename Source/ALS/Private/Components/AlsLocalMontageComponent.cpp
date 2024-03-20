#include "Components/AlsLocalMontageComponent.h"

#include "AlsCharacter.h"
#include "CharacterTasks/AlsLocalMontageTask.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsLocalMontageComponent)

void UAlsLocalMontageComponent::BeginPlay()
{
	Super::BeginPlay();

	InstancedLocalMontageTasks.Reset();
	LocalMontageTagsMask.Reset();
	for(auto& KeyValue : LocalMontageTaskClassMap)
	{
		LocalMontageTagsMask.AddTag(KeyValue.Key);
	}
}

UAlsLocalMontageTask* UAlsLocalMontageComponent::ChangeLocalMontageTaskIfNeeded(const FGameplayTag& LocalMontageTag)
{
	if (CurrentLocalMontageTask.IsValid())
	{
		if (CurrentLocalMontageTag == LocalMontageTag)
		{
			return CurrentLocalMontageTask.Get();
		}
		else
		{
			CurrentLocalMontageTask->Cancel();
			CurrentLocalMontageTask.Reset();
			CurrentLocalMontageTag = FGameplayTag::EmptyTag;
		}
	}

	if (LocalMontageTaskClassMap.Contains(LocalMontageTag))
	{
		if (InstancedLocalMontageTasks.Contains(LocalMontageTag))
		{
			CurrentLocalMontageTask = InstancedLocalMontageTasks[LocalMontageTag];
		}
		else
		{
			auto* NewTask{NewObject<UAlsLocalMontageTask>(Character.Get(), LocalMontageTaskClassMap[LocalMontageTag])};
			NewTask->Component = this;
			InstancedLocalMontageTasks.Add(LocalMontageTag, NewTask);
			CurrentLocalMontageTask = NewTask;
			CurrentLocalMontageTask->OnRegister();
		}
		CurrentLocalMontageTag = LocalMontageTag;
		CurrentLocalMontageTask->Begin();
	}

	return CurrentLocalMontageTask.Get();
}

void UAlsLocalMontageComponent::OnEndTask(UAlsLocalMontageTask* Task)
{
	if (CurrentLocalMontageTask == Task)
	{
		CurrentLocalMontageTask.Reset();
		CurrentLocalMontageTag = FGameplayTag::EmptyTag;
	}
}

void UAlsLocalMontageComponent::OnRefresh_Implementation(float DeltaTime)
{
	Super::OnRefresh_Implementation(DeltaTime);

	FGameplayTagContainer Container;
	Character->GetOwnedGameplayTags(Container);
	auto CurrentOverrideTags{Container.Filter(LocalMontageTagsMask)};

	ChangeLocalMontageTaskIfNeeded(CurrentOverrideTags.First());

	if (CurrentLocalMontageTask.IsValid())
	{
		CurrentLocalMontageTask->Refresh(DeltaTime);
	}
}

void UAlsLocalMontageComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	Super::OnControllerChanged_Implementation(PreviousController, NewController);
	if (CurrentLocalMontageTask.IsValid())
	{
		CurrentLocalMontageTask->OnControllerChanged(PreviousController, NewController);
	}
}
