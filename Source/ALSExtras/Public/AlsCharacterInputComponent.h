#pragma once

#include "AlsCharacterComponent.h"
#include "AlsInputAction.h"
#include "Utility/AlsGameplayTags.h"
#include "AlsCharacterInputComponent.generated.h"

struct FInputActionValue;
class UInputMappingContext;

UCLASS(AutoExpandCategories = ("AlsCharacterInput|Settings"))
class ALSEXTRAS_API UAlsCharacterInputComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> LookMouseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ProneAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> RagdollAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> RotationModeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ViewModeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SwitchShoulderAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsCharacterInput|Settings", Meta = (DisplayThumbnail = false))
	TArray<FAlsInputAction> InputActions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCharacterInput|Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookUpMouseSensitivity{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCharacterInput|Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookRightMouseSensitivity{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCharacterInput|Settings", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookUpRate{90.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCharacterInput|Settings", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookRightRate{240.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCharacterInput|Settings")
	FGameplayTag RagdollActionTag{AlsLocomotionActionTags::Dying};

protected:
	virtual void OnControllerChanged_Implementation(AController* PreviousController, AController* NewController) override;

	virtual void OnSetupPlayerInputComponent_Implementation(UInputComponent* Input) override;

private:
	void Input_OnLookMouse(const FInputActionValue& ActionValue);

	void Input_OnLook(const FInputActionValue& ActionValue);

	void Input_OnMove(const FInputActionValue& ActionValue);

	void Input_OnSprint(const FInputActionValue& ActionValue);

	void Input_OnWalk();

	void Input_OnCrouch();

	void Input_OnProne();

	void Input_OnJump(const FInputActionValue& ActionValue);

	void Input_OnAim(const FInputActionValue& ActionValue);

	void Input_OnRagdoll();

	void Input_OnRotationMode();

	void Input_OnViewMode();

	void Input_OnSwitchShoulder();
};
