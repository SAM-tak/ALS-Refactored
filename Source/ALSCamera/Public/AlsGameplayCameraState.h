#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptInterface.h"
#include "Utility/AlsCameraGameplayTags.h"
#include "Core/CameraVariableTableFwd.h"
#include "AlsGameplayCameraState.generated.h"

class UAlsGameplayCameraStateSettings;
class AAlsCharacter;
class UGameplayCameraComponentBase;
class UFloatCameraVariable;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class ALSCAMERA_API UAlsGameplayCameraState : public UObject
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAlsGameplayCameraStateSettings> Settings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<AAlsCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UGameplayCameraComponentBase> GameplayCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector CameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator CameraRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UPrimitiveComponent> MovementBasePrimitive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FName MovementBaseBoneName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PivotMovementBaseRelativeLagLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FQuat CameraMovementBaseRelativeRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "%"))
	float TraceDistanceRatio{1.0f};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocalLength{500.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bIsFocusPawn : 1 {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag ViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	FGameplayTag DesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Replicated)
	FGameplayTag ConfirmedDesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag PreviousConfirmedDesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "s"))
	float ViewModeChangeBlockTime{0.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInAutoFPP : 1 {false};
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Replicated)
	FGameplayTag ShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag PreviousShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<UCameraShakeBase> CurrentADSCameraShake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float FirstPersonFactor{0.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float TanHalfVfov{0.57f}; // ≒tan(60°/2)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator LastFullAimSightRot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bFullAim : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator SightRotOffset;

protected:

	FCameraVariableID FirstPersonFactorVariableId;

	void RefreshTanHalfFov(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void SetUp(AAlsCharacter* NewCharacter, UGameplayCameraComponentBase* NewGameplayCameraComponent);

	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void SetFirstPersonFactorVariable(UFloatCameraVariable* FirstPersonFactorVariable);

	UFUNCTION(BlueprintPure, Category = "ALS|Gameplay Camera State", Meta = (ReturnDisplayName = "Camera Location"))
	FVector GetFirstPersonCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Gameplay Camera State", Meta = (ReturnDisplayName = "Camera Location"))
	FVector GetEyeCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Gameplay Camera State", Meta = (ReturnDisplayName = "Pivot Location"))
	FVector GetThirdPersonPivotLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Gameplay Camera State", Meta = (ReturnDisplayName = "Trace Start"))
	FVector GetThirdPersonTraceStartLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Gameplay Camera State", Meta = (ReturnDisplayName = "Focus Location"))
	FVector GetCurrentFocusLocation() const;

	float GetTanHalfVfov() const;

	// Desired View Mode

public:
	const FGameplayTag& GetDesiredViewMode() const;

	void SetDesiredViewMode(const FGameplayTag& NewDesiredViewMode);

	const FGameplayTag& GetConfirmedDesiredViewMode() const;

protected:
	void SetConfirmedDesiredViewMode(const FGameplayTag& NewDesiredViewMode);

private:
	UFUNCTION(Server, Unreliable)
	void ServerSetConfirmedDesiredViewMode(const FGameplayTag& NewDesiredViewMode);

	// ShoulderMode

public:
	const FGameplayTag& GetShoulderMode() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void SetShoulderMode(const FGameplayTag& NewShoulderMode);

	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void ToggleShoulder();

	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void Tick(float DeltaTime);

private:
	UFUNCTION(Server, Unreliable)
	void ServerSetShoulderMode(const FGameplayTag& NewShoulderMode);

private:
	FVector CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, float DeltaTime);

	bool TryAdjustLocationBlockedByGeometry(FVector& Location, bool bDisplayDebugCameraTraces) const;

	void UpdateAimingFirstPersonCamera(float AimingAmount, const FRotator& TargetRotation);

	void UpdateFocalLength();

	void UpdateADSCameraShake(float FirstPersonOverride, float AimingAmount);

	// Overlap

	mutable TArray<FOverlapResult> Overlaps;

#if !UE_BUILD_SHIPPING
	// Debug

public:
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation);

private:
	static void DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor,
		float Scale, float HorizontalLocation, float& VerticalLocation);

	void DisplayDebugState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugShapes(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugTraces(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
#endif // !UE_BUILD_SHIPPING
};

inline FVector UAlsGameplayCameraState::GetCurrentFocusLocation() const
{
	return CameraLocation + CameraRotation.Vector() * FocalLength;
}

inline const FGameplayTag& UAlsGameplayCameraState::GetDesiredViewMode() const
{
	return DesiredViewMode;
}

inline const FGameplayTag& UAlsGameplayCameraState::GetConfirmedDesiredViewMode() const
{
	return ConfirmedDesiredViewMode;
}

inline const FGameplayTag& UAlsGameplayCameraState::GetShoulderMode() const
{
	return ShoulderMode;
}

inline float UAlsGameplayCameraState::GetTanHalfVfov() const
{
	return TanHalfVfov;
}
