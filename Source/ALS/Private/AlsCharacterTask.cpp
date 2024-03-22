#include "AlsCharacterTask.h"

#include "AlsCharacter.h"
#include "Engine/InputDelegateBinding.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterTask)

void UAlsCharacterTask::OnRegister()
{
	if (!Character.IsValid())
	{
		Character = Cast<AAlsCharacter>(GetOuter());
		ALS_ENSURE(Character.IsValid());
	}
}

void UAlsCharacterTask::Begin()
{
	ALS_ENSURE(Character.IsValid());
	if (!bActive)
	{
		bActive = true;
		bEpilogRunningCurrently = false;
		BindInput(Character->InputComponent.Get());
		K2_OnBegin();
	}
}

void UAlsCharacterTask::Refresh(float DeltaTime)
{
	if (bActive)
	{
		K2_OnRefresh(DeltaTime);
	}
}

void UAlsCharacterTask::OnEnd(bool bWasCancelled)
{
	K2_OnEnd(bWasCancelled);
	if (Character.IsValid())
	{
		UnbindInput(Character->InputComponent.Get());
	}
}

void UAlsCharacterTask::OnFinished()
{
	K2_OnFinished();
}

void UAlsCharacterTask::End()
{
	if (bActive)
	{
		OnEnd(false);
		bEpilogRunningCurrently = IsEpilogRunning();
		bActive = false;
		if (!bEpilogRunningCurrently)
		{
			OnFinished();
		}
	}
	else if(bEpilogRunningCurrently)
	{
		bEpilogRunningCurrently = IsEpilogRunning();
		if (!bEpilogRunningCurrently)
		{
			OnFinished();
		}
	}
}

void UAlsCharacterTask::Cancel()
{
	if (bActive)
	{
		OnEnd(true);
		bActive = false;
		bEpilogRunningCurrently = IsEpilogRunning();
		bActive = false;
		if (!bEpilogRunningCurrently)
		{
			OnFinished();
		}
	}
}

bool UAlsCharacterTask::IsEpilogRunning_Implementation() const
{
	return false;
}

void UAlsCharacterTask::OnControllerChanged(AController* PreviousController, AController* NewController)
{
	if (IsValid(PreviousController) && IsValid(PreviousController->InputComponent))
	{
		UnbindInput(PreviousController->InputComponent);
	}
	if (IsValid(NewController) && IsValid(NewController->InputComponent))
	{
		BindInput(NewController->InputComponent);
	}
}

void UAlsCharacterTask::BindInput(UInputComponent* InputComponent)
{
	if(bEnableInputBinding && !bInputBinded && IsValid(InputComponent))
	{
		UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent, this);
		bInputBinded = true;
	}
}

void UAlsCharacterTask::UnbindInput(UInputComponent* InputComponent)
{
	if(bInputBinded && IsValid(InputComponent))
	{
		InputComponent->ClearBindingsForObject(this);
	}
	bInputBinded = false;
}
