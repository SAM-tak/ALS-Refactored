#pragma once

#include "Components/PawnComponent.h"
#include "Utility/AlsCameraGameplayTags.h"
#include "Core/CameraVariableTableFwd.h"
#include "AlsGameplayCameraStateComponent.generated.h"

class UAlsGameplayCameraStateSettings;
class AAlsCharacter;
class UGameplayCameraComponentBase;
class UFloatCameraVariable;
class UVector3dCameraVariable;
class URotator3dCameraVariable;

UCLASS(Blueprintable, ClassGroup=Camera, meta=(BlueprintSpawnableComponent))
class ALSCAMERA_API UAlsGameplayCameraStateComponent : public UPawnComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAlsGameplayCameraStateSettings> Settings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "State")
	TWeakObjectPtr<AAlsCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UGameplayCameraComponentBase> GameplayCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector CameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator CameraRotation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocalLength{500.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bIsFocusPawn : 1{false};

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
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Replicated)
	FGameplayTag ShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag PreviousShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float FirstPersonFactor{0.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float TraceSphreRadius{10.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float TanHalfVfov{0.57f}; // ≒tan(60°/2)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector SightLocationOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FQuat SightRotationOffset;

protected:

	FCameraVariableID FirstPersonFactorVariableId;
	FCameraVariableID TraceSphreRadiusVariableId;
	FCameraVariableID FirstPersonLocationVariableId;
	FCameraVariableID EyeLocationVariableId;
	FCameraVariableID ADSLocationVariableId;
	FCameraVariableID ADSRotationVariableId;
	FVector3d BoomOffset{FVector3d::ZeroVector};
	FVector3d CenterShoulderOffset{FVector3d::ZeroVector};
	FVector3d LeftShoulderOffset{FVector3d::ZeroVector};
	FVector3d RightShoulderOffset{FVector3d::ZeroVector};

	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void Activate(bool bReset = false) override;

	virtual void Deactivate() override;

	//virtual void RegisterComponentTickFunctions(bool bRegister) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void InitializeByCameraVariables(UVector3dCameraVariable* BoomOffsetVariable,
		UVector3dCameraVariable* CenterShoulderOffsetVariable,
		UVector3dCameraVariable* LeftShoulderOffsetVariable,
		UVector3dCameraVariable* RightShoulderOffsetVariable);

	UFUNCTION(BlueprintCallable, Category = "ALS|Gameplay Camera State")
	void BindCameraVariables(UFloatCameraVariable* FirstPersonFactorVariable,
		UFloatCameraVariable* TraceSphreRadiusVariable,
		UVector3dCameraVariable* FirstPersonLocationVariable,
		UVector3dCameraVariable* EyeLocationVariable,
		UVector3dCameraVariable* ADSLocationVariable,
		URotator3dCameraVariable* ADSRotationVariable);

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
	void UpdateState(float DeltaTime);

private:
	UFUNCTION(Server, Unreliable)
	void ServerSetShoulderMode(const FGameplayTag& NewShoulderMode);

private:
	void UpdateViewMode();

	void UpdateFocalLength();

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

inline FVector UAlsGameplayCameraStateComponent::GetCurrentFocusLocation() const
{
	return CameraLocation + CameraRotation.Vector() * FocalLength;
}

inline const FGameplayTag& UAlsGameplayCameraStateComponent::GetDesiredViewMode() const
{
	return DesiredViewMode;
}

inline const FGameplayTag& UAlsGameplayCameraStateComponent::GetConfirmedDesiredViewMode() const
{
	return ConfirmedDesiredViewMode;
}

inline const FGameplayTag& UAlsGameplayCameraStateComponent::GetShoulderMode() const
{
	return ShoulderMode;
}

inline float UAlsGameplayCameraStateComponent::GetTanHalfVfov() const
{
	return TanHalfVfov;
}
