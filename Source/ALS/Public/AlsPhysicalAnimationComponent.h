// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "AlsPhysicalAnimationComponent.generated.h"

class AAlsCharacter;
class USkeletalBodySetup;

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationCurveValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftArm{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightArm{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftHand{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightHand{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftLeg{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightLeg{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockLeftFoot{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float LockRightFoot{0.0f};

	void Refresh(AAlsCharacter* Character);

	float GetLockedValue(const FName& BoneName) const;
};

USTRUCT(BlueprintType)
struct ALS_API FAlsRagdollingSettings
{
	GENERATED_BODY()
	
	FAlsRagdollingSettings();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAlsLinkedAnimationInstance> OverrideAnimLayersClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> GroundTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TEnumAsByte<ECollisionChannel>> GroundTraceResponseChannels{ECC_WorldStatic, ECC_WorldDynamic, ECC_Destructible};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsAbility, Transient)
	FCollisionResponseContainer GroundTraceResponses{ECR_Ignore};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AlsAbility, Meta = (ClampMin = 0, ForceUnits = "s"))
	float StartBlendTime{0.25f};

	// If checked, it stops the physical simulation and returns control of the bone to kinematic
	// when the conditions mentioned later are met.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility)
	uint8 bAllowFreeze : 1{false};

	// The time until it freezes forcibly after landing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility, Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedForForceFreezing{5.0f};

	// The time until it forcibly freezes after the root bone is considered to have stopped when landing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility, Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedAndStoppedForForceFreezing{1.0f};

	// When the speed is below this value, the root bone is considered to be stopped.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility, Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float RootBoneSpeedConsideredAsStopped{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility, Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float SpeedThresholdToFreeze{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility, Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "deg"))
	float AngularSpeedThresholdToFreeze{45.0f};
};

USTRUCT(BlueprintType)
struct ALS_API FAlsRagdollingState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "deg"))
	float LyingDownYawAngleDelta{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	uint8 bGrounded : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	uint8 bFacingUpward : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "s"))
	float ElapsedTime{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "s"))
	float TimeAfterGrounded{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "s"))
	float TimeAfterGroundedAndStopped{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector PrevActorLocation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	uint8 bFreezing : 1{false};

	// Speed of root bone
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "cm/s"))
	float RootBoneSpeed{0.0f};

	// Highest speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "cm/s"))
	float MaxBoneSpeed{0.0f};

	// Highest angular speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "deg"))
	float MaxBoneAngularSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bOnGroundedAndAgedFired : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bCancelRequested : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bPreviousGrounded : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UAlsLinkedAnimationInstance> OverrideAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAlsRagdollingAnimInstance* RagdollingAnimInstance{nullptr};

	FVector_NetQuantize TargetLocation;

	FAlsRagdollingSettings* Settings;
	AAlsCharacter* Character;

	void Start(AAlsCharacter* NewCharacter, FAlsRagdollingSettings& NewSettings);

	void Tick(float DeltaTime);

	void End();

	FVector TraceGround();

	bool IsGroundedAndAged() const;
};

/**
 * PhysicalAnimationComponent for ALS Refactored
 */
UCLASS()
class ALS_API UAlsPhysicalAnimationComponent : public UPhysicalAnimationComponent
{
	GENERATED_UCLASS_BODY()
	
protected:
	// The blend time Of physics blend Weight on activate physics body.
	// Not used when ragdolling activate. Ragdolling start with weight 1.0 immediately.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicalAnimation|Settings", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnActivate{0.1f};

	// The blend time Of physics blend Weight on deactivate physics body.
	// Not used when ragdolling deactivate. In the case of a ragdoll, the weight becomes zero immediately.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicalAnimation|Settings", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnDeactivate{0.1f};

	// A mask of GameplayTags used to determine the Profile. The order in the list is used as a priority.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicalAnimation|Settings")
	TArray<FGameplayTagContainer> GameplayTagMasks{
		FGameplayTagContainer{AlsLocomotionActionTags::Root},
		FGameplayTagContainer{AlsLocomotionModeTags::Root},
		FGameplayTagContainer{AlsStanceTags::Root},
		FGameplayTagContainer{AlsGaitTags::Root},
		FGameplayTagContainer{AlsOverlayModeTags::Root},
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicalAnimation|Settings")
	TMap<FGameplayTag, FAlsRagdollingSettings> RagdollingSettings;

	// Name list of PhysicalAnimationProfile Name for override.
	// Only bodies with physical animation parameters set in any of the profiles in the list will be subject to physical simulation,
	// and the simulation for other bodies will be turned off.
	// Physical animation parameters are applied in order from the beginning of the list,
	// and if multiple parameters are set for the same body in different profiles,
	// they are overwritten by the later parameters in the list.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation)
	TArray<FName> OverrideProfileNames;

	// Name list of PhysicalAnimationProfile Name for multiply.
	// 'Multiply' means only overwriting the physical animation parameters,
	// without affecting the on/off state of the physical simulation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation)
	TArray<FName> MultiplyProfileNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicalAnimation|State", Transient)
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicalAnimation|State", Transient)
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	TArray<FName> CurrentProfileNames;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	TArray<FName> CurrentMultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	FGameplayTagContainer CurrentGameplayTags;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	FGameplayTagContainer PreviousGameplayTags;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	FAlsPhysicalAnimationCurveValues CurveValues;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	uint8 bActive : 1{false};

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	uint8 bRagdolling : 1{false};

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	TArray<FGameplayTag> ActivationRequest;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	FGameplayTag CurrentRagdolling;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient)
	FAlsRagdollingState RagdollingState;

	UPROPERTY(VisibleAnywhere, Category = "PhysicalAnimation|State", Transient, Replicated)
	FVector_NetQuantize RagdollingTargetLocation;

protected:
	virtual void OnRegister() override;

	virtual void OnRefresh(float DeltaTime);

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation);

	bool IsRagdolling() const;

	UFUNCTION(BlueprintPure, Category = "ALS|PhysicalAnimation")
	bool IsRagdollingAndGroundedAndAged() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|PhysicalAnimation")
	void RequestActivation(const FGameplayTag &AbilityTag);

	UFUNCTION(BlueprintPure, Category = "ALS|PhysicalAnimation")
	bool IsBoneUnderSimulation(const FName& BoneName) const;

	const FAlsRagdollingState& GetRagdollingState() const
	{
		return RagdollingState;
	}

private:
	void ClearGameplayTags();

	void RefreshBodyState(float DelaTime);

	bool IsProfileExist(const FName& ProfileName) const;

	bool HasAnyProfile(const class USkeletalBodySetup* BodySetup) const;

	bool NeedsProfileChange();

	void SelectProfile();
};
