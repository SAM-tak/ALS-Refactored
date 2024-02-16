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
 * 
 */
UCLASS()
class ALS_API UAlsPhysicalAnimationComponent : public UPhysicalAnimationComponent
{
	GENERATED_BODY()
	
protected:
	// The blend time Of physics blend Weight on activate physics body.
	// Not used when ragdolling activate. Ragdolling start with weight 1.0 immediately.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation, Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnActivate{0.1f};

	// The blend time Of physics blend Weight on deactivate physics body.
	// Not used when ragdolling deactivate. In the case of a ragdoll, the weight becomes zero immediately.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation, Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendTimeOfBlendWeightOnDeactivate{0.1f};

	/** Name list of PhysicalAnimationProfile Name for override.
	  Only bodies with physical animation parameters set in any of the profiles in the list will be subject to physical simulation,
	  and the simulation for other bodies will be turned off.
	  Physical animation parameters are applied in order from the beginning of the list,
	  and if multiple parameters are set for the same body in different profiles,
	  they are overwritten by the later parameters in the list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation)
	TArray<FName> OverrideProfileNames;

	/** Name list of PhysicalAnimationProfile Name for multiply.
	  'Multiply' means only overwriting the physical animation parameters,
	  without affecting the on/off state of the physical simulation.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalAnimation)
	TArray<FName> MultiplyProfileNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|PhysicalAnimation", Transient)
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|PhysicalAnimation", Transient)
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	TArray<FName> CurrentProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	TArray<FName> CurrentMultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	TArray<FName> CurrentOverrideMultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	FGameplayTag LocomotionAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	FGameplayTag LocomotionMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	FGameplayTag OverlayMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	TArray<FName> OverrideMultiplyProfileNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	uint8 bActive : 1 {false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|PhysicalAnimation", Transient)
	uint8 bRagdolling : 1 {false};

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation);

private:
	bool NeedsProfileChange(const AAlsCharacter* Character);

	void ClearGameplayTags();

	void Refresh(float DelaTime);

	float GetLockedValue(const FAlsPhysicalAnimationCurveState& Curves, const FName& BoneName);

	bool IsProfileExist(const FName& ProfileName);

	bool HasAnyProfile(const class USkeletalBodySetup* BodySetup);

	void SelectProfile(const AAlsCharacter* Character);
};
