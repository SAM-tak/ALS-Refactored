#include "Components/AlsLocalMontageComponent.h"

#include "AlsCharacter.h"
#include "AlsMotionWarpingComponent.h"
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

UAlsLocalMontageTask* UAlsLocalMontageComponent::Play(const FGameplayTag& LocalMontageTag)
{
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);
	
	if (Character->HasServerRole())
	{
		MulticastPlay(LocalMontageTag);
	}

	return PlayImplementation(LocalMontageTag);
}

void UAlsLocalMontageComponent::MulticastPlay_Implementation(const FGameplayTag& LocalMontageTag)
{
	if (Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		PlayImplementation(LocalMontageTag);
	}
}

UAlsLocalMontageTask* UAlsLocalMontageComponent::PlayImplementation(const FGameplayTag& LocalMontageTag)
{
	if (CurrentLocalMontageTask.IsValid())
	{
		CurrentLocalMontageTask->Cancel();
		CurrentLocalMontageTask.Reset();
		CurrentLocalMontageTag = FGameplayTag::EmptyTag;
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

		if (CurrentLocalMontageTask.IsValid())
		{
			CurrentLocalMontageTag = LocalMontageTag;
			CurrentLocalMontageTask->Begin();
		}
		else
		{
			UE_LOG(LogAls, Error, TEXT("UAlsLocalMontageComponent : Correspond Local Montage Task was not found for '%s'"), *LocalMontageTag.ToString());
		}
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

void UAlsLocalMontageComponent::AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation,
																									  FRotator TargetRotation)
{
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	Character->GetMotionWarping()->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);

	if (Character->HasServerRole())
	{
		MulticastAddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
	}
}

void UAlsLocalMontageComponent::MulticastAddOrUpdateWarpTargetFromLocationAndRotation_Implementation(FName WarpTargetName, FVector_NetQuantize TargetLocation,
																									 FRotator TargetRotation)
{
	if (Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		Character->GetMotionWarping()->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
	}
}
