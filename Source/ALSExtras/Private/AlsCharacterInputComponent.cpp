#include "AlsCharacterInputComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "AlsCharacter.h"
#include "AlsAbilitySystemComponent.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"
#include "InputTriggers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterInputComponent)

void UAlsCharacterInputComponent::OnRegister()
{
	Super::OnRegister();

	if (Character.IsValid())
	{
		Character->OnSetupPlayerInputComponent.AddUObject(this, &ThisClass::OnSetupPlayerInputComponent);
	}
}

void UAlsCharacterInputComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	auto* PreviousPlayerController{Cast<APlayerController>(PreviousController)};
	if (IsValid(PreviousPlayerController))
	{
		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayerController->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}

	auto* NewPlayerController{Cast<APlayerController>(NewController)};
	if (IsValid(NewPlayerController))
	{
		NewPlayerController->InputYawScale_DEPRECATED = 1.0f;
		NewPlayerController->InputPitchScale_DEPRECATED = 1.0f;
		NewPlayerController->InputRollScale_DEPRECATED = 1.0f;

		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayerController->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			FModifyContextOptions Options;
			Options.bNotifyUserSettings = true;

			InputSubsystem->AddMappingContext(InputMappingContext, 0, Options);
		}
	}

	Super::OnControllerChanged_Implementation(PreviousController, NewController);
}

void UAlsCharacterInputComponent::OnSetupPlayerInputComponent_Implementation(UInputComponent* Input)
{
	auto* EnhancedInput{Cast<UEnhancedInputComponent>(Input)};
	if (IsValid(EnhancedInput))
	{
		auto* AlsAbilitySystem{Character->GetAlsAbilitySystem()};
		for(auto& AbilityInputAction : AbilityInputActions)
		{
			AbilityInputAction.BindAction(AlsAbilitySystem, EnhancedInput);
		}
	}
}
