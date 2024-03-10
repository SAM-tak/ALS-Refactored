#include "AlsCharacterExampleComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "AlsCharacter.h"
#include "AlsCameraMovementComponent.h"
#include "AlsAbilitySystemComponent.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterExampleComponent)

void UAlsCharacterExampleComponent::OnRegister()
{
	Super::OnRegister();

	if (Character.IsValid())
	{
		CameraMovementComponent = Character->FindComponentByClass<UAlsCameraMovementComponent>();
	}
}

void UAlsCharacterExampleComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
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

	auto* NewPlayerController{ Cast<APlayerController>(NewController) };
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

		CameraMovementComponent->Activate(true);
	}
	else
	{
		CameraMovementComponent->Deactivate();
	}

	Super::OnControllerChanged_Implementation(PreviousController, NewController);
}

void UAlsCharacterExampleComponent::OnSetupPlayerInputComponent_Implementation(UInputComponent* Input)
{
	Super::OnSetupPlayerInputComponent_Implementation(Input);

	auto* EnhancedInput{Cast<UEnhancedInputComponent>(Input)};
	if (IsValid(EnhancedInput))
	{
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLookMouse);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLook);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMove);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSprint);
		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnWalk);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnCrouch);
		EnhancedInput->BindAction(ProneAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnProne);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnJump);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnAim);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnFire);
		EnhancedInput->BindAction(RagdollAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRagdoll);
		EnhancedInput->BindAction(RollAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRoll);
		EnhancedInput->BindAction(RotationModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRotationMode);
		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnViewMode);
		EnhancedInput->BindAction(SwitchShoulderAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSwitchShoulder);
	}
}

void UAlsCharacterExampleComponent::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const auto Value{ActionValue.Get<FVector2D>()};

	Character->AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	Character->AddControllerYawInput(Value.X * LookRightMouseSensitivity);
}

void UAlsCharacterExampleComponent::Input_OnLook(const FInputActionValue& ActionValue)
{
	const auto Value{ActionValue.Get<FVector2D>()};

	Character->AddControllerPitchInput(Value.Y * LookUpRate);
	Character->AddControllerYawInput(Value.X * LookRightRate);
}

void UAlsCharacterExampleComponent::Input_OnMove(const FInputActionValue& ActionValue)
{
	const auto Value{UAlsMath::ClampMagnitude012D(ActionValue.Get<FVector2D>())};

	const auto ForwardDirection{UAlsMath::AngleToDirectionXY(UE_REAL_TO_FLOAT(Character->GetViewState().Rotation.Yaw))};
	const auto RightDirection{UAlsMath::PerpendicularCounterClockwiseXY(ForwardDirection)};

	Character->AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void UAlsCharacterExampleComponent::Input_OnSprint(const FInputActionValue& ActionValue)
{
	Character->SetDesiredGait(ActionValue.Get<bool>() ? AlsDesiredGaitTags::Sprinting : AlsDesiredGaitTags::Running);
}

void UAlsCharacterExampleComponent::Input_OnWalk()
{
	if (Character->GetDesiredGait() == AlsDesiredGaitTags::Walking)
	{
		Character->SetDesiredGait(AlsDesiredGaitTags::Running);
	}
	else if (Character->GetDesiredGait() == AlsDesiredGaitTags::Running)
	{
		Character->SetDesiredGait(AlsDesiredGaitTags::Walking);
	}
}

void UAlsCharacterExampleComponent::Input_OnCrouch()
{
	if (Character->GetDesiredStance() != AlsDesiredStanceTags::Crouching)
	{
		Character->SetDesiredStance(AlsDesiredStanceTags::Crouching);
	}
	else if (Character->GetDesiredStance() == AlsDesiredStanceTags::Crouching)
	{
		Character->SetDesiredStance(AlsDesiredStanceTags::Standing);
	}
}

void UAlsCharacterExampleComponent::Input_OnProne()
{
	Character->SetDesiredStance(AlsDesiredStanceTags::LyingFront);
}

void UAlsCharacterExampleComponent::Input_OnJump(const FInputActionValue& ActionValue)
{
	if (ActionValue.Get<bool>())
	{
		if (Character->HasMatchingGameplayTag(AlsLocomotionModeTags::Grounded))
		{
			if (Character->GetAlsAbilitySystem()->TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::GettingUp))
			{
				return;
			}		
			if (Character->GetAlsAbilitySystem()->TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::Mantling))
			{
				return;
			}
		}

		if (Character->GetDesiredStance() != AlsDesiredStanceTags::Standing)
		{
			Character->SetDesiredStance(AlsDesiredStanceTags::Standing);
			return;
		}

		Character->Jump();
	}
	else
	{
		Character->StopJumping();
	}
}

void UAlsCharacterExampleComponent::Input_OnAim(const FInputActionValue& ActionValue)
{
	Character->SetAimingMode(ActionValue.Get<bool>() ? AlsAimingModeTags::AimDownSight : FGameplayTag::EmptyTag);
}

void UAlsCharacterExampleComponent::Input_OnFire(const FInputActionValue& ActionValue)
{
	auto FireTag{FGameplayTag::RequestGameplayTag(FName{TEXT("AlsExtra.Action.Fire")})};
	if (Character->HasMatchingGameplayTag(FireTag))
	{
		Character->GetAlsAbilitySystem()->CancelAbilitiesBySingleTag(FireTag);
	}
	else
	{
		Character->GetAlsAbilitySystem()->TryActivateAbilitiesBySingleTag(FireTag);
	}
}

void UAlsCharacterExampleComponent::Input_OnRagdoll()
{
	if (Character->HasMatchingGameplayTag(AlsLocomotionActionTags::GettingDown))
	{
		Character->GetAlsAbilitySystem()->TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::GettingUp);
	}
	else if (Character->HasMatchingGameplayTag(RagdollActionTag))
	{
		Character->GetAlsAbilitySystem()->CancelAbilitiesBySingleTag(RagdollActionTag);
	}
	else
	{
		Character->GetAlsAbilitySystem()->TryActivateAbilitiesBySingleTag(RagdollActionTag);
	}
}

void UAlsCharacterExampleComponent::Input_OnRoll()
{
	Character->GetAlsAbilitySystem()->TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::Rolling);
}

void UAlsCharacterExampleComponent::Input_OnRotationMode()
{
	Character->SetDesiredRotationMode(Character->GetDesiredRotationMode() == AlsDesiredRotationModeTags::VelocityDirection
									  ? AlsDesiredRotationModeTags::ViewDirection
									  : AlsDesiredRotationModeTags::VelocityDirection);
}

void UAlsCharacterExampleComponent::Input_OnViewMode()
{
	Character->SetDesiredViewMode(Character->GetDesiredViewMode() == AlsDesiredViewModeTags::ThirdPerson
								  ? AlsDesiredViewModeTags::FirstPerson
								  : AlsDesiredViewModeTags::ThirdPerson);
}

void UAlsCharacterExampleComponent::Input_OnSwitchShoulder()
{
	Character->ToggleRightShoulder();
}
