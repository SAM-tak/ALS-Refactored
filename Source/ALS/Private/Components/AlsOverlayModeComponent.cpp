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
	if (CurrentOverlayTask.IsValid())
	{
		CurrentOverlayTask->End();
		if (CurrentOverlayTask->HasFinished())
		{
			CurrentOverlayTask.Reset();
		}
	}

	if (!CurrentOverlayTask.IsValid() && OverlayClassMap.Contains(OverlayMode))
	{
		if (InstancedOverlayTasks.Contains(OverlayMode))
		{
			CurrentOverlayTask = InstancedOverlayTasks[OverlayMode];
		}
		else
		{
			auto* NewTask{NewObject<UAlsOverlayTask>(Character.Get(), OverlayClassMap[OverlayMode])};
			NewTask->Component = this;
			InstancedOverlayTasks.Add(OverlayMode, NewTask);
			CurrentOverlayTask = NewTask;
			NewTask->OnRegister();
		}
		CurrentOverlayTask->Begin();
	}
}

void UAlsOverlayModeComponent::OnRefresh_Implementation(float DeltaTime)
{
	Super::OnRefresh_Implementation(DeltaTime);
	if (CurrentOverlayTask.IsValid())
	{
		CurrentOverlayTask->Refresh(DeltaTime);
	}
}

void UAlsOverlayModeComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	Super::OnControllerChanged_Implementation(PreviousController, NewController);
	if (CurrentOverlayTask.IsValid())
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
