#include "AlsCharacterExample.h"

#include "AlsCameraSkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Utility/AlsMath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterExample)

AAlsCharacterExample::AAlsCharacterExample()
{
	//CameraMovement = CreateDefaultSubobject<UAlsCameraMovementComponent>(FName{TEXTVIEW("CameraMovement")});

	CameraSkeletalMesh = CreateDefaultSubobject<UAlsCameraSkeletalMeshComponent>(FName{TEXTVIEW("CameraSkeletalMesh")});
	CameraSkeletalMesh->SetRelativeRotation_Direct({0.0f, 90.0f, 0.0f});

	Camera = CreateDefaultSubobject<UCameraComponent>(FName{TEXTVIEW("Camera")});
	Camera->PrimaryComponentTick.bStartWithTickEnabled = false;
	Camera->PrimaryComponentTick.TickGroup = TG_PostPhysics;
	Camera->bAutoActivate = false;
	Camera->bTickInEditor = false;
	Camera->bHiddenInGame = true;
	CameraSkeletalMesh->SetCameraComponent(Camera);
}

void AAlsCharacterExample::NotifyControllerChanged()
{
	const auto* PreviousPlayer{Cast<APlayerController>(PreviousController)};
	if (IsValid(PreviousPlayer))
	{
		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayer->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}

	auto* NewPlayer{Cast<APlayerController>(GetController())};
	if (IsValid(NewPlayer))
	{
		NewPlayer->InputYawScale_DEPRECATED = 1.0f;
		NewPlayer->InputPitchScale_DEPRECATED = 1.0f;
		NewPlayer->InputRollScale_DEPRECATED = 1.0f;

		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayer->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			FModifyContextOptions Options;
			Options.bNotifyUserSettings = true;

			InputSubsystem->AddMappingContext(InputMappingContext, 0, Options);
		}

		CameraSkeletalMesh->Activate(true);
	}
	else
	{
		CameraSkeletalMesh->Deactivate();
	}

	Super::NotifyControllerChanged();
}

void AAlsCharacterExample::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

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
		EnhancedInput->BindAction(RagdollAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRagdoll);
		EnhancedInput->BindAction(RollAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRoll);
		EnhancedInput->BindAction(RotationModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRotationMode);
		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnViewMode);
		EnhancedInput->BindAction(SwitchShoulderAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSwitchShoulder);
	}
}

void AAlsCharacterExample::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const auto Value{ActionValue.Get<FVector2D>()};

	AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	AddControllerYawInput(Value.X * LookRightMouseSensitivity);
}

void AAlsCharacterExample::Input_OnLook(const FInputActionValue& ActionValue)
{
	const auto Value{ActionValue.Get<FVector2D>()};

	AddControllerPitchInput(Value.Y * LookUpRate);
	AddControllerYawInput(Value.X * LookRightRate);
}

void AAlsCharacterExample::Input_OnMove(const FInputActionValue& ActionValue)
{
	const auto Value{UAlsMath::ClampMagnitude012D(ActionValue.Get<FVector2D>())};

	const auto ForwardDirection{UAlsMath::AngleToDirectionXY(UE_REAL_TO_FLOAT(GetViewState().Rotation.Yaw))};
	const auto RightDirection{UAlsMath::PerpendicularCounterClockwiseXY(ForwardDirection)};

	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void AAlsCharacterExample::Input_OnSprint(const FInputActionValue& ActionValue)
{
	SetDesiredGait(ActionValue.Get<bool>() ? AlsDesiredGaitTags::Sprinting : AlsDesiredGaitTags::Running);
}

void AAlsCharacterExample::Input_OnWalk()
{
	if (GetDesiredGait() == AlsDesiredGaitTags::Walking)
	{
		SetDesiredGait(AlsDesiredGaitTags::Running);
	}
	else if (GetDesiredGait() == AlsDesiredGaitTags::Running)
	{
		SetDesiredGait(AlsDesiredGaitTags::Walking);
	}
}

void AAlsCharacterExample::Input_OnCrouch()
{
	if (GetDesiredStance() != AlsDesiredStanceTags::Crouching)
	{
		SetDesiredStance(AlsDesiredStanceTags::Crouching);
	}
	else if (GetDesiredStance() == AlsDesiredStanceTags::Crouching)
	{
		SetDesiredStance(AlsDesiredStanceTags::Standing);
	}
}

void AAlsCharacterExample::Input_OnProne()
{
	SetDesiredStance(AlsDesiredStanceTags::Lying);
}

void AAlsCharacterExample::Input_OnJump(const FInputActionValue& ActionValue)
{
	if (ActionValue.Get<bool>())
	{
		if (StopRagdolling())
		{
			return;
		}

		if (StartMantlingGrounded())
		{
			return;
		}

		if (GetDesiredStance() != AlsDesiredStanceTags::Standing)
		{
			SetDesiredStance(AlsDesiredStanceTags::Standing);
			return;
		}

		Jump();
	}
	else
	{
		StopJumping();
	}
}

void AAlsCharacterExample::Input_OnAim(const FInputActionValue& ActionValue)
{
	SetAimingMode(ActionValue.Get<bool>() ? AlsAimingModeTags::AimDownSight : FGameplayTag::EmptyTag);
}

void AAlsCharacterExample::Input_OnRagdoll()
{
	if (!StopRagdolling())
	{
		StartRagdolling();
	}
}

void AAlsCharacterExample::Input_OnRoll()
{
	static constexpr auto PlayRate{1.3f};

	StartRolling(PlayRate);
}

void AAlsCharacterExample::Input_OnRotationMode()
{
	SetDesiredRotationMode(GetDesiredRotationMode() == AlsDesiredRotationModeTags::VelocityDirection
						   ? AlsDesiredRotationModeTags::ViewDirection
						   : AlsDesiredRotationModeTags::VelocityDirection);
}

void AAlsCharacterExample::Input_OnViewMode()
{
	SetDesiredViewMode(GetDesiredViewMode() == AlsDesiredViewModeTags::ThirdPerson ? AlsDesiredViewModeTags::FirstPerson : AlsDesiredViewModeTags::ThirdPerson);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AAlsCharacterExample::Input_OnSwitchShoulder()
{
	CameraSkeletalMesh->SetRightShoulder(!CameraSkeletalMesh->IsRightShoulder());
}

void AAlsCharacterExample::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation)
{
	if (CameraSkeletalMesh->IsActive())
	{
		CameraSkeletalMesh->DisplayDebug(Canvas, DisplayInfo, VerticalLocation);
	}

	Super::DisplayDebug(Canvas, DisplayInfo, Unused, VerticalLocation);
}
