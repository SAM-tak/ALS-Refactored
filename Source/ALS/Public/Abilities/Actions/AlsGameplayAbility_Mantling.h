// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Actions/AlsGameplayAbility_MontageBase.h"
#include "AlsGameplayAbility_Mantling.generated.h"

class UAnimMontage;
class UCurveFloat;

UENUM(BlueprintType)
enum class EAlsMantlingType : uint8
{
	High,
	Medium,
	Low,
	InAir
};

USTRUCT(BlueprintType)
struct ALS_API FAlsMantlingParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FVector_NetQuantize100 TargetRelativeLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FRotator TargetRelativeRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "cm"))
	float MantlingHeight{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	EAlsMantlingType MantlingType{EAlsMantlingType::High};
};

UCLASS(Blueprintable, BlueprintType)
class ALS_API UAlsMantlingSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAnimMontage> Montage;

	// If checked, mantling will automatically calculate the start time based on how much vertical
	// distance the character needs to move to reach the object they are about to mantle.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0))
	uint8 bAutoCalculateStartTime : 1{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "!bAutoCalculateStartTime"))
	FVector2f StartTimeReferenceHeight{50.0f, 100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "!bAutoCalculateStartTime"))
	FVector2f StartTime{0.5f, 0.0f};

	// Optional mantling time to horizontal correction amount curve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UCurveFloat> HorizontalCorrectionCurve;

	// Optional mantling time to vertical correction amount curve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UCurveFloat> VerticalCorrectionCurve;
};

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float TraceAngleThreshold{110.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float MaxReachAngle{50.0f};

	// Prevents mantling on surfaces whose slope angle exceeds this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float SlopeAngleThreshold{35.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability", AdvancedDisplay, Meta = (ClampMin = 0, ClampMax = 1))
	float SlopeAngleThresholdCos{FMath::Cos(FMath::DegreesToRadians(35.0f))};

	// If a dynamic object has a speed bigger than this value, then do not start mantling.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ForceUnits = "cm/s"))
	float TargetPrimitiveSpeedThreshold{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ForceUnits = "cm"))
	float MantlingHighHeightThreshold{125.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ForceUnits = "cm"))
	float MantlingMediumHeightThreshold{75.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	FAlsMantlingTraceSettings GroundedTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	FAlsMantlingTraceSettings InAirTrace{{50.0f, 150.0f}, 70.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	TEnumAsByte<ECollisionChannel> MantlingTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	TArray<TEnumAsByte<ECollisionChannel>> MantlingTraceResponseChannels{ECC_WorldStatic, ECC_WorldDynamic, ECC_Destructible};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Ability", AdvancedDisplay)
	FCollisionResponseContainer MantlingTraceResponses{ECR_Ignore};

	// Used when the mantling was interrupted and we need to stop the animation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendOutDuration{0.3f};

	// If checked, ragdolling will start if the object the character is mantling on was destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability")
	uint8 bStartRagdollingOnTargetPrimitiveDestruction : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ability", Meta = (EditCondition = "bStartRagdollingOnTargetPrimitiveDestruction"))
	FGameplayTag TryActiveOnPrimitiveDestruction{AlsLocomotionActionTags::FreeFalling};

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Ability|State", Transient)
	int32 RootMotionSourceId{0};

	float CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, const float MantlingHeight) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Als|Ability|Mantling")
	void Tick(const float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "Als|Ability|Mantling")
	bool CanMantleByParameter(const FGameplayAbilityActorInfo& ActorInfo, const FAlsMantlingParameters& Parameter) const;

	UFUNCTION(BlueprintCallable, Category = "Als|Ability|Mantling")
	void CommitParameter(const FGameplayAbilitySpecHandle Handle, const FAlsMantlingParameters& Parameters) const;

	UFUNCTION(BlueprintCallable, Category = "Als|Ability|Mantling")
	bool CanMantle(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FAlsMantlingParameters& MantlingParameters) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Als|Ability|Mantling")
	UAlsMantlingSettings* SelectMantlingSettings(const FAlsMantlingParameters& Parameters) const;

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	static TMap<FGameplayAbilitySpecHandle, FAlsMantlingParameters> ParameterMap;

	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;
};
