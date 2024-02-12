#pragma once

#include "NativeGameplayTags.h"
#include "AlsPhysicalAnimationState.generated.h"

class USkeletalMeshComponent;
struct FAlsPhysicalAnimationCurveState;
struct FAlsPhysicalAnimationSettings;

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Transient)
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Transient)
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	TArray<FName> ProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	TArray<FName> MultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	FGameplayTag LocomotionAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	FGameplayTag LocomotionMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	FGameplayTag OverlayMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	TArray<FName> OverrideMultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	uint8 bActive : 1 {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS", Transient)
	uint8 bRagdolling : 1 {false};

	bool NeedsProfielChange(const FGameplayTag& NewLocomotionMode, const FGameplayTag& NewLocomotionAction, const FGameplayTag& NewStance,
							const FGameplayTag& NewGait, const FGameplayTag& NewOverlayMode, const TArray<FName>& NewOverrideMultiplyProfileNames);

	void ClearAlsTags();

	void Refresh(float DelaTime, USkeletalMeshComponent* Mesh, const FAlsPhysicalAnimationSettings& Settings, const FAlsPhysicalAnimationCurveState& Curves);

private:
	bool HasAnyProfile(const class USkeletalBodySetup* BodySetup);
};
