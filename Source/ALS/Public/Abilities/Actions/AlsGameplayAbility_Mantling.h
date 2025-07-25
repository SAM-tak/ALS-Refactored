// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Actions/AlsGameplayAbility_MontageBase.h"
#include "AlsGameplayAbility_Mantling.generated.h"

class UAnimMontage;
class UCurveFloat;
class UAlsMantlingSettings;
class UAlsRootMotionComponent;

USTRUCT(BlueprintType)
struct ALS_API FAlsMantlingTraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FVector2f LedgeHeight{50.0f, 225.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float ReachDistance{75.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float TargetLocationOffset{15.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float StartLocationOffset{55.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	uint8 bDrawFailedTraces : 1{false};
};

/**
 * Ragdolling
 */
UCLASS(Abstract)
class ALS_API UAlsGameplayAbility_Mantling : public UAlsGameplayAbility_MontageBase
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float TraceAngleThreshold{110.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float MaxReachAngle{50.0f};

	// Prevents mantling on surfaces whose slope angle exceeds this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float SlopeAngleThreshold{35.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", AdvancedDisplay, Meta = (ClampMin = 0, ClampMax = 1))
	float SlopeAngleThresholdCos{FMath::Cos(FMath::DegreesToRadians(35.0f))};

	// If a dynamic object has a speed bigger than this value, then do not start mantling.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (ForceUnits = "cm/s"))
	float TargetPrimitiveSpeedThreshold{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (ForceUnits = "cm"))
	float MantlingHighHeightThreshold{125.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (ForceUnits = "cm"))
	float MantlingMediumHeightThreshold{75.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling")
	FAlsMantlingTraceSettings GroundedTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling")
	FAlsMantlingTraceSettings InAirTrace{{50.0f, 150.0f}, 70.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling")
	TEnumAsByte<ECollisionChannel> MantlingTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling")
	TArray<TEnumAsByte<ECollisionChannel>> MantlingTraceResponseChannels{ECC_WorldStatic, ECC_WorldDynamic, ECC_Destructible};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", AdvancedDisplay)
	FCollisionResponseContainer MantlingTraceResponses{ECR_Ignore};

	// If checked, ragdolling will start if the object the character is mantling on was destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling")
	uint8 bStartRagdollingOnTargetPrimitiveDestruction : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Mantling", Meta = (EditCondition = "bStartRagdollingOnTargetPrimitiveDestruction"))
	FGameplayTag TryActiveOnPrimitiveDestruction{AlsLocomotionActionTags::FreeFalling};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsAbility|Mantling|State", Transient)
	TWeakObjectPtr<UAlsRootMotionComponent> RootMotionComponent;

protected:
	float CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, const float MantlingHeight) const;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Mantling")
	void Tick(const float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Mantling")
	bool CanMantleByParameter(const FGameplayAbilityActorInfo& ActorInfo, const FAlsMantlingParameters& Parameter) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Mantling")
	void CommitParameter(const FGameplayAbilitySpecHandle Handle, const FAlsMantlingParameters& Parameters) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Mantling")
	bool CanMantle(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FAlsMantlingParameters& MantlingParameters) const;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	static TMap<FGameplayAbilitySpecHandle, FAlsMantlingParameters> ParameterMap;

	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;
};
