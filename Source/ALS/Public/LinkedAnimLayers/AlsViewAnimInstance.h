#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "State/AlsViewAnimationState.h"
#include "AlsViewAnimInstance.generated.h"

UCLASS(Abstract, AutoExpandCategories = ("ALS|Settings"))
class ALS_API UAlsViewAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Settings", Meta = (ClampMin = 0))
	float LookTowardsCameraRotationInterpolationSpeed{8.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Settings", Meta = (ClampMin = 0))
	float LookTowardsInputYawAngleInterpolationSpeed{8.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	FRotator Rotation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = -90, ClampMax = 90, ForceUnits = "deg"))
	float PitchAngle{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float PitchAmount{0.5f}; // using in overlay

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float LookAmount{1.0f}; // using

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	FAlsSpineRotationState SpineRotation; // using SpineRotation.YawAngle in UAlsAnimationInstance::GetControlRigInput 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	FAlsLookState Look; // using YawAngle PitchAngle YawLeftAmount YawRightAmount YawForwardAmount

protected:
	UFUNCTION(BlueprintCallable, Category = "ALS|Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ReinitializeLook();

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void RefreshLook();

public:
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
};
