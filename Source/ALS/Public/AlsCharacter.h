#pragma once

#include "GameFramework/Character.h"
#include "State/AlsLocomotionState.h"
#include "State/AlsMovementBaseState.h"
#include "State/AlsViewState.h"
#include "Utility/AlsGameplayTags.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Abilities/AlsAbilitySet.h"
#include "AlsCharacter.generated.h"

struct FAlsMantlingParameters;
struct FAlsMantlingTraceSettings;
class UAlsCharacterMovementComponent;
class UAlsCharacterSettings;
class UAlsPhysicalAnimationComponent;
class UAlsMovementSettings;
class UAlsAnimationInstance;
class UAlsMantlingSettings;
class UAlsAbilitySystemComponent;
class UAlsMotionWarpingComponent;

DECLARE_EVENT_TwoParams(AAlsCharacter, FAlsCharacter_OnContollerChanged, AController*, AController*);

DECLARE_EVENT_OneParam(AAlsCharacter, FAlsCharacter_OnSetupPlayerInputComponent, UInputComponent*);

DECLARE_EVENT_FourParams(AAlsCharacter, FAlsCharacter_OnDebugDisplayDelegate, UCanvas*, const FDebugDisplayInfo&, float&, float&);

DECLARE_EVENT_OneParam(AAlsCharacter, FAlsCharacter_OnRefresh, float);

DECLARE_EVENT_OneParam(AAlsCharacter, FAlsCharacter_OnChangeGameplayTag, const FGameplayTag &);

UCLASS(Abstract, AutoExpandCategories = ("Als Character|Settings"))
class ALS_API AAlsCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface
{
	GENERATED_UCLASS_BODY()

	friend UAlsPhysicalAnimationComponent;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Als Character|State", Transient)
	TObjectPtr<UAlsCharacterMovementComponent> AlsCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Als Character|State", Transient)
	TWeakObjectPtr<UAlsAnimationInstance> AnimationInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character")
	TObjectPtr<UAlsPhysicalAnimationComponent> PhysicalAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character")
	TObjectPtr<UAlsAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character")
	TObjectPtr<UAlsMotionWarpingComponent> MotionWarping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings")
	TObjectPtr<UAlsCharacterSettings> Settings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings")
	TObjectPtr<UAlsMovementSettings> MovementSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Als Character|Settings")
	TObjectPtr<UAlsAbilitySet> AbilitySet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings|Desired State", Replicated)
	FGameplayTag DesiredRotationMode{AlsDesiredRotationModeTags::ViewDirection};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings|Desired State", Replicated)
	FGameplayTag DesiredStance{AlsDesiredStanceTags::Standing};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings|Desired State", Replicated)
	FGameplayTag DesiredGait{AlsDesiredGaitTags::Running};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings|Desired State", Replicated)
	FGameplayTag DesiredViewMode{AlsDesiredViewModeTags::ThirdPerson};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings|Desired State", ReplicatedUsing = OnReplicated_OverlayMode)
	FGameplayTag OverlayMode{AlsOverlayModeTags::Default};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FGameplayTag LocomotionMode{AlsLocomotionModeTags::Grounded};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FGameplayTag RotationMode{AlsRotationModeTags::ViewDirection};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FGameplayTag Stance{AlsStanceTags::Standing};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FGameplayTag Gait{AlsGaitTags::Walking};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FGameplayTag ViewMode{AlsDesiredViewModeTags::ThirdPerson};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FAlsMovementBaseState MovementBase;

	// Replicated raw view rotation. Depending on the context, this rotation can be in world space, or in movement
	// base space. In most cases, it is better to use FAlsViewState::Rotation to take advantage of network smoothing.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient, ReplicatedUsing = OnReplicated_ReplicatedViewRotation)
	FRotator ReplicatedViewRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FAlsViewState ViewState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient, Replicated,
			  Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FAlsLocomotionState LocomotionState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Als Character|State", Transient)
	FRotator PendingFocalRotationRelativeAdjustment{ForceInit};

	FTimerHandle BrakingFrictionFactorResetTimer;

public:
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreRegisterAllComponents() override;

	virtual void PostRegisterAllComponents() override;

	virtual void PostInitializeComponents() override;

	FORCEINLINE UAlsCharacterMovementComponent* GetAlsCharacterMovement() const { return AlsCharacterMovement; }

	FORCEINLINE UAlsAnimationInstance* GetAlsAnimationInstace() const { return AnimationInstance.Get(); }

	/** Name of the PhysicalAnimationComponent. */
	static FName PhysicalAnimationComponentName;

	/** Returns PhysicalAnimation subobject **/
	template <class T>
	FORCEINLINE_DEBUGGABLE T* GetPhysicalAnimation() const
	{
		return CastChecked<T>(PhysicalAnimation, ECastCheckedType::NullAllowed);
	}
	FORCEINLINE UAlsPhysicalAnimationComponent* GetPhysicalAnimation() const { return PhysicalAnimation; }

	/** Name of the PhysicalAnimationComponent. */
	static FName AbilitySystemComponentName;

	FORCEINLINE UAlsAbilitySystemComponent* GetAlsAbilitySystem() const { return AbilitySystem; }

	/** Name of the MotionWarpingComponent. */
	static FName MotionWarpingComponentName;

	FORCEINLINE UAlsMotionWarpingComponent* GetMotionWarping() const { return MotionWarping; }

protected:
	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

public:
	FAlsCharacter_OnContollerChanged OnContollerChanged;

	FAlsCharacter_OnSetupPlayerInputComponent OnSetupPlayerInputComponent;

	FAlsCharacter_OnRefresh OnRefresh;

	virtual void PostNetReceiveLocationAndRotation() override;

	virtual void OnRep_ReplicatedBasedMovement() override;

	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void Restart() override;

	// IAbilitySystemInterface

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// IGameplayTagAssetInterface

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	void ReplaceAlsAbilitySystem(UAlsAbilitySystemComponent *NewAbilitySystem);

private:
	mutable FGameplayTagContainer TempTagContainer;

	void RefreshMeshProperties() const;

	void RefreshMovementBase();

	// Desired View Mode

public:
	const FGameplayTag& GetDesiredViewMode() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredViewMode"))
	void SetDesiredViewMode(const FGameplayTag& NewDesiredViewMode);

private:
	UFUNCTION(Server, Unreliable)
	void ServerSetDesiredViewMode(const FGameplayTag& NewViewMode);

	// View Mode

public:
	const FGameplayTag& GetViewMode() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character")
	void SetViewMode(const FGameplayTag& NewViewMode);

protected:

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	void OnViewModeChanged(const FGameplayTag& PreviousViewMode);

	// Locomotion Mode

public:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode = 0) override;

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	const FGameplayTag& GetLocomotionMode() const;

protected:
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);

	UFUNCTION()
	virtual void NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	void OnLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);

	// Desired Rotation Mode

public:
	const FGameplayTag& GetDesiredRotationMode() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredRotationMode"))
	void SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);

	// Rotation Mode

public:
	const FGameplayTag& GetRotationMode() const;

protected:
	void SetRotationMode(const FGameplayTag& NewRotationMode);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	void OnRotationModeChanged(const FGameplayTag& PreviousRotationMode);

	void RefreshRotationMode();

	// Desired Stance

public:
	const FGameplayTag& GetDesiredStance() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredStance"))
	void SetDesiredStance(const FGameplayTag& NewDesiredStance);

protected:
	virtual void ApplyDesiredStance();

	// Stance

public:
	virtual bool CanCrouch() const override;

	virtual void Crouch(bool bClientSimulation = false) override;

	virtual void UnCrouch(bool bClientSimulation = false) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

public:
	const FGameplayTag& GetStance() const;

protected:
	void SetStance(const FGameplayTag& NewStance);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	void OnStanceChanged(const FGameplayTag& PreviousStance);

	// Desired Gait

public:
	const FGameplayTag& GetDesiredGait() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredGait"))
	void SetDesiredGait(const FGameplayTag& NewDesiredGait);

private:
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredGait(const FGameplayTag& NewDesiredGait);

	// Gait

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	const FGameplayTag& GetGait() const;

protected:
	void SetGait(const FGameplayTag& NewGait);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	void OnGaitChanged(const FGameplayTag& PreviousGait);

private:
	void RefreshGait();

	FGameplayTag CalculateMaxAllowedGait() const;

	FGameplayTag CalculateActualGait(const FGameplayTag& MaxAllowedGait) const;

	bool CanSprint() const;

	// Overlay Mode

public:
	const FGameplayTag& GetOverlayMode() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewOverlayMode"))
	void SetOverlayMode(const FGameplayTag& NewOverlayMode);

	FAlsCharacter_OnChangeGameplayTag OnOverlayModeChanged;

private:
	void SetOverlayMode(const FGameplayTag& NewOverlayMode, bool bSendRpc);

	UFUNCTION(Client, Reliable)
	void ClientSetOverlayMode(const FGameplayTag& NewOverlayMode);

	UFUNCTION(Server, Reliable)
	void ServerSetOverlayMode(const FGameplayTag& NewOverlayMode);

	UFUNCTION()
	void OnReplicated_OverlayMode(const FGameplayTag& PreviousOverlayMode);

protected:

	// Locomotion Action

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	FGameplayTag GetLocomotionAction() const;

	// Input

public:
	const FVector& GetInputDirection() const;

protected:
	void SetInputDirection(FVector NewInputDirection);

	virtual void RefreshInput(float DeltaTime);

	// Controll Rotation Adjustment

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Character")
	void SetFocalRotation(const FRotator& NewFocalRotation);

private:
	void TryAdjustControllRotation(float DeltaTime);

	// View

public:
	virtual FRotator GetViewRotation() const override;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character")
	void SetLookRotation(const FRotator& NewLookRotation);

private:
	void SetReplicatedViewRotation(const FRotator& NewViewRotation, bool bSendRpc);

	UFUNCTION(Server, Unreliable)
	void ServerSetReplicatedViewRotation(const FRotator& NewViewRotation);

	UFUNCTION()
	void OnReplicated_ReplicatedViewRotation();

	FRotator TargetLookRotation{NAN, NAN, NAN};

public:
	void CorrectViewNetworkSmoothing(const FRotator& NewTargetRotation, bool bRelativeTargetRotation);

public:
	const FAlsViewState& GetViewState() const;

private:
	void RefreshView(float DeltaTime);

	void RefreshViewNetworkSmoothing(float DeltaTime);

	// Locomotion

public:
	const FAlsLocomotionState& GetLocomotionState() const;

private:
	void SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle);

	void RefreshLocomotionLocationAndRotation();

	void RefreshLocomotionEarly();

	void RefreshLocomotion(float DeltaTime);

	void RefreshLocomotionLate(float DeltaTime);

	// Jumping

public:
	virtual void Jump() override;

	virtual void OnJumped_Implementation() override;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnJumpedNetworked();

	void OnJumpedNetworked();

	// Rotation

public:
	virtual void FaceRotation(FRotator Rotation, float DeltaTime) override final;

	void RefreshRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None);

private:
	void RefreshGroundedRotation(float DeltaTime);

protected:
	virtual bool RefreshCustomGroundedMovingRotation(float DeltaTime);

	virtual bool RefreshCustomGroundedNotMovingRotation(float DeltaTime);

	float CalculateGroundedMovingRotationInterpolationSpeed() const;

	void RefreshGroundedAimingRotation(float DeltaTime);

	bool RefreshConstrainedAimingRotation(float DeltaTime, bool bApplySecondaryConstraint = false);

private:
	void ApplyRotationYawSpeedAnimationCurve(float DeltaTime);

	void RefreshInAirRotation(float DeltaTime);

protected:
	virtual bool RefreshCustomInAirRotation(float DeltaTime);

	void RefreshInAirAimingRotation(float DeltaTime);

	void RefreshRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);

	void RefreshRotationExtraSmooth(float TargetYawAngle, float DeltaTime,
	                                float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);

	void RefreshTargetYawAngleUsingLocomotionRotation();

	void RefreshTargetYawAngle(float TargetYawAngle);

	void RefreshViewRelativeTargetYawAngle();

	// ADS

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	float GetAimAmount() const;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	bool HasSight() const;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Character")
	void GetSightLocAndRot(FVector& Loc, FRotator& Rot) const;

	// Utility

public:
	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	const FGameplayTag& DesiredToActual(const FGameplayTag& SourceTag) const;

	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	bool IsCharacterSelf() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Character")
	bool HasServerRole() const;

	// Others

public:
	bool IsLied() const;

	void SetIsLied(bool bNewIsLied);

	UFUNCTION(BlueprintCallable, Category = Character)
	virtual bool CanLie() const;

	UFUNCTION(BlueprintCallable, Category = Character)
	virtual void Lie();

	virtual void OnStartLie(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust);

	virtual void OnEndLie(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnStartLie", ScriptName = "OnStartLie"))
	void K2_OnStartLie(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnEndLie", ScriptName = "OnEndLie"))
	void K2_OnEndLie(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Als Character|Settings", Meta = (ForceUnits = "s"))
	float CapsuleUpdateSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Als Character|State", replicatedUsing = OnRep_IsLied)
	uint32 bIsLied : 1;

private:
	void RefreshCapsuleSize(float DeltaTime);

	void UpdateCapsule(float DeltaTime, float EyeHeight, float EyeHeightSpeed, float HalfHeight, float HalfHeightSpeed, float Radius, float RadiusSpeed);

	/** Handle Lying replicated from server */
	UFUNCTION()
	virtual void OnRep_IsLied();

#if !UE_BUILD_SHIPPING
	// Debug

public:
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation) override;

	FAlsCharacter_OnDebugDisplayDelegate OnDisplayDebug;

private:
	static void DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor,
	                               float Scale, float HorizontalLocation, float& VerticalLocation);

	static TArray<FName> CurveNames;

	static void InitializeCurveNames();

	void DisplayDebugCurves(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugShapes(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugTraces(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugMantling(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
#endif // !UE_BUILD_SHIPPING
};

inline void AAlsCharacter::ReplaceAlsAbilitySystem(UAlsAbilitySystemComponent* NewAbilitySystem)
{
	AbilitySystem = NewAbilitySystem;
}

inline const FGameplayTag& AAlsCharacter::GetDesiredViewMode() const
{
	return DesiredViewMode;
}

inline const FGameplayTag& AAlsCharacter::GetDesiredRotationMode() const
{
	return DesiredRotationMode;
}

inline const FGameplayTag& AAlsCharacter::GetDesiredStance() const
{
	return DesiredStance;
}

inline const FGameplayTag& AAlsCharacter::GetDesiredGait() const
{
	return DesiredGait;
}

inline const FGameplayTag& AAlsCharacter::GetLocomotionMode() const
{
	return LocomotionMode;
}

inline const FGameplayTag& AAlsCharacter::GetViewMode() const
{
	return ViewMode;
}

inline const FGameplayTag& AAlsCharacter::GetRotationMode() const
{
	return RotationMode;
}

inline const FGameplayTag& AAlsCharacter::GetStance() const
{
	return Stance;
}

inline const FGameplayTag& AAlsCharacter::GetGait() const
{
	return Gait;
}

inline const FGameplayTag& AAlsCharacter::GetOverlayMode() const
{
	return OverlayMode;
}

inline const FVector& AAlsCharacter::GetInputDirection() const
{
	return InputDirection;
}

inline const FAlsViewState& AAlsCharacter::GetViewState() const
{
	return ViewState;
}

inline const FAlsLocomotionState& AAlsCharacter::GetLocomotionState() const
{
	return LocomotionState;
}

inline bool AAlsCharacter::IsLied() const
{
	return bIsLied;
}
