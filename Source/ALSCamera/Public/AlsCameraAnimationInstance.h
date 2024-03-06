#pragma once

#include "Animation/AnimInstance.h"
#include "Utility/AlsGameplayTags.h"
#include "AlsCameraAnimationInstance.generated.h"

class AAlsCharacter;
class UAlsCameraMovementComponent;

UCLASS()
class ALSCAMERA_API UAlsCameraAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<AAlsCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsCameraMovementComponent> CameraMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTagContainer CurrentGameplayTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bRightShoulder : 1{true};

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;
};
