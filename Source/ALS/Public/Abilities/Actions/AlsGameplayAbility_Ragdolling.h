// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/AlsGameplayAbility_Action.h"
#include "AlsGameplayAbility_Ragdolling.generated.h"

class UAlsLinkedAnimationInstance;

/**
 * Ragdolling
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Ragdolling : public UAlsGameplayAbility_Action
{
	GENERATED_UCLASS_BODY()

	friend class UAlsPhysicalAnimationComponent;
	friend class UAlsRagdollingAnimInstance;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AlsAbility)
	TSubclassOf<UAlsLinkedAnimationInstance> OverrideAnimLayersClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility)
	TEnumAsByte<ECollisionChannel> GroundTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlsAbility)
	TArray<TEnumAsByte<ECollisionChannel>> GroundTraceResponseChannels{ECC_WorldStatic, ECC_WorldDynamic, ECC_Destructible};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AlsAbility)
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "deg"))
	float LyingDownYawAngleDelta{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient)
	uint8 bGrounded : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient)
	uint8 bFacingUpward : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "s"))
	float ElapsedTime{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "s"))
	float TimeAfterGrounded{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "s"))
	float TimeAfterGroundedAndStopped{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient)
	FVector PrevActorLocation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient)
	uint8 bFreezing : 1{false};

	// Speed of root bone
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "cm/s"))
	float RootBoneSpeed{0.0f};

	// Highest speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "cm/s"))
	float MaxBoneSpeed{0.0f};

	// Highest angular speed among all bodies
	// Exists solely for display in the editor. Useful when determining setting value.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Meta = (ForceUnits = "deg"))
	float MaxBoneAngularSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|State", Transient, Replicated)
	FVector_NetQuantize TargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	uint8 bOnGroundedAndAgedFired : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	uint8 bCancelRequested : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	uint8 bPreviousGrounded : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	TWeakObjectPtr<UAlsLinkedAnimationInstance> OverrideAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|State", Transient)
	UAlsRagdollingAnimInstance* RagdollingAnimInstance{nullptr};

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	// In OnGroundedAndAged or Tick context, calls EndAbility or CancelAbility makes stuttering.
	// Then use this instead.
	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Ragdolling")
	void RequestCancel();

	UFUNCTION(BlueprintPure, Category = "ALS|Ability|Ragdolling")
	bool IsGroundedAndAged() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION()
	void Tick(const float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Ability|Ragdolling", DisplayName = "Tick", Meta = (ScriptName = "Tick"))
	void K2_OnTick(const float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Ability|Ragdolling", DisplayName = "On Grounded And Aged", Meta = (ScriptName = "OnGroundedAndAged"))
	void K2_OnGroundedAndAged();

	void Cancel();

	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;

	void SetTargetLocation(const FVector& NewTargetLocation);

	UFUNCTION(Server, Unreliable)
	void ServerSetTargetLocation(const FVector_NetQuantize& NewTargetLocation);

	FVector TraceGround();
};
