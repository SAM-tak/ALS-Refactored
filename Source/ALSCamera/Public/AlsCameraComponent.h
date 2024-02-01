#pragma once

#include "Camera/CameraComponent.h"
#include "Utility/AlsMath.h"
#include "AlsCameraComponent.generated.h"

class USkeletalMeshComponent;
class UAlsCameraSettings;
class AAlsCharacter;

UCLASS()
class ALSCAMERA_API UAlsCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAlsCameraSettings> Settings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UCameraShakeBase> ADSCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, ClampMax = 1))
	float ADSCameraShakeScale{0.2f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (ClampMin = 0, ClampMax = 1))
	float PostProcessWeight{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	TObjectPtr<USkeletalMeshComponent> CameraSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<AAlsCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ForceUnits = "x"))
	float PreviousGlobalTimeDilation{1.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PivotTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PivotLagLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector CameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator CameraRotation;

	UPROPERTY(BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<UPrimitiveComponent> MovementBasePrimitive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FName MovementBaseBoneName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PivotMovementBaseRelativeLagLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FQuat CameraMovementBaseRelativeRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector CurrentLeadVector{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator LatestCameraYawRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "%"))
	float TraceDistanceRatio{1.0f};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FocalLength{500.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag PreviousViewMode{AlsViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bFPP : 1 {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInAutoFPP : 1 {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bRightShoulder : 1 {true};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bPreviousRightShoulder : 1 {true};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<UCameraShakeBase> CurrentADSCameraShake;

public:
	UAlsCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnRegister() override;

	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	virtual void Activate(bool bReset) override;

	//virtual void InitAnim(bool bForceReinitialize) override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//virtual void CompleteParallelAnimationEvaluation(bool bDoPostAnimationEvaluation) override;

	void SetCameraSkeletalMesh(USkeletalMeshComponent* NewCameraSkeletalMesh);

public:
	//float GetPostProcessWeight() const;

	//void SetPostProcessWeight(float NewPostProcessWeight);
	UFUNCTION(BlueprintPure, Category = "ALS|Camera", meta = (Keywords = "AnimBlueprint", UnsafeDuringActorConstruction = "true"))
	class UAnimInstance* GetAnimInstance() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera")
	bool IsRightShoulder() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	void SetRightShoulder(bool bNewRightShoulder);

	UFUNCTION(BlueprintPure, Category = "ALS|Camera")
	bool IsFirstPerson() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera", Meta = (ReturnDisplayName = "Camera Location"))
	FVector GetFirstPersonCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera", Meta = (ReturnDisplayName = "Camera Location"))
	FVector GetEyeCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera", Meta = (ReturnDisplayName = "Pivot Location"))
	FVector GetThirdPersonPivotLocation() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera", Meta = (ReturnDisplayName = "Trace Start"))
	FVector GetThirdPersonTraceStartLocation() const;

	//UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	//void GetViewInfo(FMinimalViewInfo& ViewInfo) const;

	UFUNCTION(BlueprintPure, Category = "ALS|Camera", Meta = (ReturnDisplayName = "Focus Location"))
	FVector GetCurrentFocusLocation() const;

private:
	void TickCamera(float DeltaTime, bool bAllowLag = true);

	FRotator CalculateCameraRotation(const FRotator& CameraTargetRotation, float DeltaTime, bool bAllowLag) const;

	FVector CalculatePivotLagLocation(const FQuat& CameraYawRotation, float DeltaTime, bool bAllowLag) const;

	FVector CalculatePivotOffset() const;

	FVector CalculateCameraOffset() const;

	FVector CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, float DeltaTime, bool bAllowLag);

	bool TryAdjustLocationBlockedByGeometry(FVector& Location, bool bDisplayDebugCameraTraces) const;

	void CalculateAimingFirstPersonCamera(float AimingAmount, const FRotator& TargetRotation);

	float CalculateFocalLength() const;

	void UpdateADSCameraShake(float FirstPersonOverride, float AimingAmount);

	// Overlap

	mutable TArray<FOverlapResult> Overlaps;

	// Debug

public:
	void DisplayDebug(const UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& VerticalLocation) const;

private:
	static void DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor,
	                               float Scale, float HorizontalLocation, float& VerticalLocation);

	void DisplayDebugCurves(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugShapes(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugTraces(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
};
//
//inline float UAlsCameraComponent::GetPostProcessWeight() const
//{
//	return PostProcessWeight;
//}
//
//inline void UAlsCameraComponent::SetPostProcessWeight(const float NewPostProcessWeight)
//{
//	PostProcessWeight = UAlsMath::Clamp01(NewPostProcessWeight);
//}

inline bool UAlsCameraComponent::IsRightShoulder() const
{
	return bRightShoulder;
}

inline void UAlsCameraComponent::SetRightShoulder(const bool bNewRightShoulder)
{
	bRightShoulder = bNewRightShoulder;
}

inline FVector UAlsCameraComponent::GetCurrentFocusLocation() const
{
	return CameraLocation + CameraRotation.Vector() * FocalLength;
}
