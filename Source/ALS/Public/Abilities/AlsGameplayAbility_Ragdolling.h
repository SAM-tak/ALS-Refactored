// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AlsGameplayAbility.h"
#include "AlsGameplayAbility_Ragdolling.generated.h"

/**
 * Ragdolling
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Ragdolling : public UAlsGameplayAbility
{
	GENERATED_UCLASS_BODY()

	friend class UAlsPhysicalAnimationComponent;
	friend class UAlsAnimationInstance;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	TArray<TEnumAsByte<ECollisionChannel>> GroundTraceResponseChannels;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability", AdvancedDisplay)
	FCollisionResponseContainer GroundTraceResponses{ECR_Ignore};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Ability", Meta = (ClampMin = 0, ForceUnits = "s"))
	float StartBlendTime{0.25f};

	// If checked, it stops the physical simulation and returns control of the bone to kinematic
	// when the conditions mentioned later are met.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	uint8 bAllowFreeze : 1{false};

	// The time until it freezes forcibly after landing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedForForceFreezing{5.0f};

	// The time until it forcibly freezes after the root bone is considered to have stopped when landing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "s"))
	float TimeAfterGroundedAndStoppedForForceFreezing{1.0f};

	// When the speed is below this value, the root bone is considered to be stopped.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float RootBoneSpeedConsideredAsStopped{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "cm/s"))
	float SpeedThresholdToFreeze{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, EditCondition = "bAllowFreeze", ForceUnits = "deg"))
	float AngularSpeedThresholdToFreeze{45.0f};

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "deg"))
	float LyingDownYawAngleDelta{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient)
	uint8 bGrounded : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient)
	uint8 bFacingUpward : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "s"))
	float ElapsedTime{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "s"))
	float TimeAfterGrounded{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "s"))
	float TimeAfterGroundedAndStopped{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient)
	FVector PrevActorLocation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient)
	uint8 bFreezing : 1{false};

	// Speed of root bone
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "cm/s"))
	float RootBoneSpeed{0.0f};

	// Highest speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "cm/s"))
	float MaxBoneSpeed{0.0f};

	// Highest angular speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ForceUnits = "deg"))
	float MaxBoneAngularSpeed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "x"))
	float FlailPlayRate{1.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability|State", Transient, Replicated)
	FVector_NetQuantize TargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Ability|State", Transient)
	uint8 bOnGroundedAndAgedFired : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Ability|State", Transient)
	uint8 bCancelRequested : 1{false};

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	// In OnGroundedAndAged or Tick context, calls EndAbility or CancelAbility makes stuttering.
	// Then use this instead.
	UFUNCTION(BlueprintCallable, Category = "Als|Ability|Ragdolling")
	void RequestCancel();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Als|Ability|Ragdolling")
	void Tick(const float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Ability|Ragdolling", DisplayName = "On Grounded And Aged", meta = (ScriptName = "OnGroundedAndAged"))
	void K2_OnGroundedAndAged();

	void Cancel();

	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;

	UFUNCTION()
	void ProcessTick(const float DeltaTime);

	void SetTargetLocation(const FVector& NewTargetLocation);

	UFUNCTION(Server, Unreliable)
	void ServerSetTargetLocation(const FVector_NetQuantize& NewTargetLocation);

	FVector TraceGround();

	UFUNCTION(BlueprintPure, Category = "Als|Ability|Ragdolling")
	bool IsGroundedAndAged() const;
};
