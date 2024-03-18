#pragma once

#include "AbilitySystemComponent.h"
#include "InputTriggers.h"
#include "AlsAbilitySystemComponent.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class AAlsCharacter;
class UAlsMantlingSettings;
struct FAlsRootMotionSource_Mantling;

USTRUCT()
struct ALS_API FAlsMantlingRootMotionParameters
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UAlsMantlingSettings> MantlingSettings;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UPROPERTY()
	FVector_NetQuantize100 TargetRelativeLocation{ForceInit};

	UPROPERTY()
	FVector_NetQuantize TargetAnimationLocation{ForceInit};

	UPROPERTY()
	FRotator TargetRelativeRotation{ForceInit};

	UPROPERTY()
	float StartTime{0.0f};
};

/**
 * AbilitySystemComponent for ALS Refactored
 */
UCLASS()
class ALS_API UAlsAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void OnRegister() override;

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|AbilitySystem")
	void OnControllerChanged(AController* PreviousController, AController* NewController);

	UFUNCTION(BlueprintNativeEvent, Category = "ALS|AbilitySystem")
	void OnRefresh(float DeltaTime);

	// Utility

public:
	inline void CancelAbilitiesBySingleTag(const FGameplayTag& Tag)
	{
		FGameplayTagContainer TagContainer{Tag};
		CancelAbilities(&TagContainer);
	}

	inline bool TryActivateAbilitiesBySingleTag(const FGameplayTag& Tag, bool bAllowRemoteActivation = true)
	{
		return TryActivateAbilitiesByTag(FGameplayTagContainer{Tag}, bAllowRemoteActivation);
	}

	// Input binding

public:
	void BindAbilityActivationInput(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action, ETriggerEvent TriggerEvent, const FGameplayTag& InputTag);

	void UnbindAbilityInputs(UEnhancedInputComponent* EnhancedInputComponent, const FGameplayTag& InputTag);

private:
	TMap<FGameplayTag, TArray<uint32>> BindingHandles;

	void ActivateOnInputAction(FGameplayTag InputTag);

	// Matling root motion support

public:
	FAlsRootMotionSource_Mantling* GetCurrentMantlingRootMotionSource() const;

	void SetMantlingRootMotion(const FAlsMantlingRootMotionParameters& Parameters);

protected:
	UFUNCTION(Server, Reliable)
	void ServerSetMantlingRootMotion(const FAlsMantlingRootMotionParameters& Parameters);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetMantlingRootMotion(const FAlsMantlingRootMotionParameters& Parameters);

	void SetMantlingRootMotionImplementation(const FAlsMantlingRootMotionParameters& Parameters);

	void OnTick_Mantling();

private:
	int32 MantlingRootMotionSourceId{0};
};
