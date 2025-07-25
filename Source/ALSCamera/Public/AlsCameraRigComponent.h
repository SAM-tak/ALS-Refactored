#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "Utility/AlsCameraGameplayTags.h"
#include "AlsCameraRigComponent.generated.h"

class UCameraComponent;
class UAlsCameraSettings;
class AAlsCharacter;

UCLASS(HideCategories = (Object, Mobility, Clothing, Physics, MasterPoseComponent, Collision, AnimationRig, Lighting, Deformer,
						 Rendering, PathTracing, HLOD, Navigation, VirtualTexture, Materials, LeaderPoseComponent,
						 Optimization, LOD, MaterialParameters, TextureStreaming, Mobile, RayTracing))
class ALSCAMERA_API UAlsCameraRigComponent : public USkeletalMeshComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|Settings")
	TObjectPtr<UAlsCameraSettings> Settings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|Settings")
	TSubclassOf<UCameraShakeBase> ADSCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCameraMovement|Settings", Meta = (ClampMin = 0, ClampMax = 1))
	float ADSCameraShakeScale{0.2f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsCameraMovement|Settings")
	FGameplayTag DesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	TWeakObjectPtr<AAlsCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	TWeakObjectPtr<UCameraComponent> TargetCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient, Meta = (ForceUnits = "x"))
	float PreviousGlobalTimeDilation{1.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FVector PivotTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FVector PivotLagLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FVector CameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FRotator CameraRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	TWeakObjectPtr<UPrimitiveComponent> MovementBasePrimitive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FName MovementBaseBoneName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FVector PivotMovementBaseRelativeLagLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FQuat CameraMovementBaseRelativeRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient, Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "%"))
	float TraceDistanceRatio{1.0f};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocalLength{500.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	uint8 bIsFocusPawn : 1 {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient, Replicated)
	FGameplayTag ConfirmedDesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FGameplayTag PreviousConfirmedDesiredViewMode{AlsCameraViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient, Meta = (ClampMin = 0, ForceUnits = "s"))
	float ViewModeChangeBlockTime{0.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	uint8 bInAutoFPP : 1 {false};
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient, Replicated)
	FGameplayTag ShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FGameplayTag PreviousShoulderMode{AlsCameraShoulderModeTags::Right};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	TObjectPtr<UCameraShakeBase> CurrentADSCameraShake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	float TanHalfVfov{0.57f}; // ≒tan(60°/2)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FRotator LastFullAimSightRot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	uint8 bFullAim : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsCameraMovement|State", Transient)
	FRotator SightRotOffset;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Activate(bool bReset = false) override;

	virtual void Deactivate() override;

	virtual void RegisterComponentTickFunctions(bool bRegister) override;

protected:
	virtual void OnRegister() override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Camera Movement")
	void OnControllerChanged(AController* PreviousController, AController* NewController);

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void CompleteParallelAnimationEvaluation(const bool bDoPostAnimationEvaluation) override;

	void RefreshTanHalfFov(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Camera Movement")
	void SetCameraComponent(UCameraComponent* NewCameraComponent);

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera Movement")
	UCameraComponent* GetCameraComponent() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Movement", Meta = (ReturnDisplayName = "Camera Location"))
	FVector GetFirstPersonCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Movement", Meta = (ReturnDisplayName = "Camera Location"))
	FVector GetEyeCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Movement", Meta = (ReturnDisplayName = "Pivot Location"))
	FVector GetThirdPersonPivotLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Movement", Meta = (ReturnDisplayName = "Trace Start"))
	FVector GetThirdPersonTraceStartLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Movement", Meta = (ReturnDisplayName = "Focus Location"))
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

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera Movement")
	void SetShoulderMode(const FGameplayTag& NewShoulderMode);

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera Movement")
	void ToggleShoulder();

private:
	UFUNCTION(Server, Unreliable)
	void ServerSetShoulderMode(const FGameplayTag& NewShoulderMode);

private:
	void TickCamera(float DeltaTime, bool bAllowLag = true);

	FRotator CalculateCameraRotation(const FRotator& CameraTargetRotation, float DeltaTime, bool bAllowLag) const;

	FVector CalculatePivotLagLocation(const FQuat& CameraYawRotation, float DeltaTime, bool bAllowLag) const;

	FVector CalculatePivotOffset() const;

	FVector CalculateCameraOffset() const;

	FVector CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, float DeltaTime, bool bAllowLag);

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

	void DisplayDebugCurves(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugShapes(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugTraces(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
#endif // !UE_BUILD_SHIPPING
};

inline FVector UAlsCameraRigComponent::GetCurrentFocusLocation() const
{
	return CameraLocation + CameraRotation.Vector() * FocalLength;
}

inline UCameraComponent* UAlsCameraRigComponent::GetCameraComponent() const
{
	return TargetCamera.Get();
}

inline const FGameplayTag& UAlsCameraRigComponent::GetDesiredViewMode() const
{
	return DesiredViewMode;
}

inline const FGameplayTag& UAlsCameraRigComponent::GetConfirmedDesiredViewMode() const
{
	return ConfirmedDesiredViewMode;
}

inline const FGameplayTag& UAlsCameraRigComponent::GetShoulderMode() const
{
	return ShoulderMode;
}

inline float UAlsCameraRigComponent::GetTanHalfVfov() const
{
	return TanHalfVfov;
}
