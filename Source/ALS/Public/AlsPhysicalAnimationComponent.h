// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "AlsPhysicalAnimationComponent.generated.h"

struct FAlsPhysicalAnimationCurveState;
class AAlsCharacter;
class USkeletalBodySetup;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation, Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnActivate{0.1f};

	// The blend time Of physics blend Weight on deactivate physics body.
	// Not used when ragdolling deactivate. In the case of a ragdoll, the weight becomes zero immediately.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation, Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnDeactivate{0.1f};

	// A mask of GameplayTags used to determine the Profile. The order in the list is used as a priority.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation)
	TArray<FGameplayTagContainer> GameplayTagMasks{
		FGameplayTagContainer{AlsLocomotionActionTags::Root},
		FGameplayTagContainer{AlsLocomotionModeTags::Root},
		FGameplayTagContainer{AlsStanceTags::Root},
		FGameplayTagContainer{AlsGaitTags::Root},
		FGameplayTagContainer{AlsOverlayModeTags::Root},
	};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|PhysicalAnimation", Transient)
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|PhysicalAnimation", Transient)
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	TArray<FName> CurrentProfileNames;

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	TArray<FName> CurrentMultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	FGameplayTagContainer CurrentGameplayTags;

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	FGameplayTagContainer PreviousGameplayTags;

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	uint8 bActive : 1 {false};

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	uint8 bRagdolling : 1 {false};

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	uint8 bRagdollingFreezed : 1 {false};

	UPROPERTY(VisibleAnywhere, Category = "State|PhysicalAnimation", Transient)
	TWeakObjectPtr<class UAlsGameplayAbility_Ragdolling> LatestRagdolling;

public:
	virtual void BeginPlay() override;

	virtual void Refresh(const AAlsCharacter* Character);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation);

private:
	void ClearGameplayTags();

	void RefreshBodyState(float DelaTime);

	static float GetLockedValue(const FAlsPhysicalAnimationCurveState& Curves, const FName& BoneName);

	bool IsProfileExist(const FName& ProfileName) const;

	bool HasAnyProfile(const class USkeletalBodySetup* BodySetup) const;

	bool NeedsProfileChange();

	void SelectProfile();

	UFUNCTION()
	void OnAbilityActivated(class UGameplayAbility* GameplayAbility);
};
