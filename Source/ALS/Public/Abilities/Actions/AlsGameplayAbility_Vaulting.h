// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Actions/AlsGameplayAbility_MontageBase.h"
#include "AlsGameplayAbility_Vaulting.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsVaultingParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FVector_NetQuantize100 TargetRelativeLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FRotator TargetRelativeRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "cm"))
	float VaultingHeight{0.0f};
};

USTRUCT(BlueprintType)
struct ALS_API FAlsVaultingTraceSettings
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
class ALS_API UAlsGameplayAbility_Vaulting : public UAlsGameplayAbility_MontageBase
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float TraceAngleThreshold{110.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float MaxReachAngle{50.0f};

	// Prevents mantling on surfaces whose slope angle exceeds this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float SlopeAngleThreshold{35.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting", AdvancedDisplay, Meta = (ClampMin = 0, ClampMax = 1))
	float SlopeAngleThresholdCos{FMath::Cos(FMath::DegreesToRadians(35.0f))};

	// If a dynamic object has a speed bigger than this value, then do not start mantling.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting", Meta = (ForceUnits = "cm/s"))
	float TargetPrimitiveSpeedThreshold{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting")
	FAlsVaultingTraceSettings TraceSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting")
	TEnumAsByte<ECollisionChannel> VaultingTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting")
	TArray<TEnumAsByte<ECollisionChannel>> VaultingTraceResponseChannels{ECC_WorldStatic, ECC_WorldDynamic, ECC_Destructible};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AlsAbility|Vaulting", AdvancedDisplay)
	FCollisionResponseContainer VaultingTraceResponses{ECR_Ignore};

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Vaulting")
	void Tick(const float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Ability|Vaulting")
	bool CanVaultByParameter(const FGameplayAbilityActorInfo& ActorInfo, const FAlsVaultingParameters& Parameter) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Vaulting")
	void CommitParameter(const FGameplayAbilitySpecHandle Handle, const FAlsVaultingParameters& Parameters) const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Ability|Vaulting")
	bool CanVault(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FAlsVaultingParameters& VaultingParameters) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Ability|Vaulting")
	FAlsPlayMontageParameter SelectMontage(const FAlsVaultingParameters& Parameters) const;

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
	static TMap<FGameplayAbilitySpecHandle, FAlsVaultingParameters> ParameterMap;

	TWeakObjectPtr<class UAlsAbilityTask_Tick> TickTask;
};
