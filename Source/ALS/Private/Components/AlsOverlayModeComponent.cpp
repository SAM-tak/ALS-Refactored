#include "Components/AlsOverlayModeComponent.h"

#include "AlsCharacter.h"
#include "LinkedAnimLayers/AlsOverlayAnimInstance.h"
#include "CharacterTasks/AlsOverlayTask.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverlayModeComponent)

void UAlsOverlayModeComponent::OnRegister()
{
	Super::OnRegister();

	if (Character.IsValid())
	{
		Character->OnOverlayModeChanged.AddUObject(this, &ThisClass::OnChangeOverlayMode);
	}
}

void UAlsOverlayModeComponent::BeginPlay()
{
	Super::BeginPlay();

	InstancedOverlayTasks.Reset();

	if (Character.IsValid())
	{
		ChangeOverlayTask(Character->GetOverlayMode());
	}
}

void UAlsOverlayModeComponent::ChangeOverlayTask(const FGameplayTag& OverlayMode)
{
	if (IsValid(CurrentOverlayTask))
	{
		CurrentOverlayTask->End();
		CurrentOverlayTask = nullptr;
	}

	if (OverlayClassMap.Contains(OverlayMode))
	{
		auto& TaskClass{OverlayClassMap[OverlayMode]};
		if (InstancedOverlayTasks.Contains(TaskClass))
		{
			CurrentOverlayTask = InstancedOverlayTasks[TaskClass].Get();
		}
		else
		{
			CurrentOverlayTask = NewObject<UAlsOverlayTask>(Character.Get(), TaskClass);
			CurrentOverlayTask->OnRegister();
			InstancedOverlayTasks.Add(TaskClass, CurrentOverlayTask);
		}
		CurrentOverlayTask->Begin();
	}
}

void UAlsOverlayModeComponent::OnRefresh_Implementation(float DeltaTime)
{
	Super::OnRefresh_Implementation(DeltaTime);
	if (IsValid(CurrentOverlayTask))
	{
		CurrentOverlayTask->Refresh(DeltaTime);
	}
}

void UAlsOverlayModeComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	Super::OnControllerChanged_Implementation(PreviousController, NewController);
	if (IsValid(CurrentOverlayTask))
	{
		CurrentOverlayTask->OnControllerChanged(PreviousController, NewController);
	}
}

void UAlsOverlayModeComponent::OnChangeOverlayMode_Implementation(const FGameplayTag& PreviousOverlayMode)
{
	if (Character.IsValid())
	{
		ChangeOverlayTask(Character->GetOverlayMode());
	}
}
