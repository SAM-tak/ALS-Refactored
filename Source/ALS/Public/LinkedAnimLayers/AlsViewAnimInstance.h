#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "AlsViewAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsLookState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bReinitializationRequired : 1{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float WorldYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -90, ClampMax = 90, ForceUnits = "deg"))
	float PitchAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 1))
	float YawForwardAmount{0.5f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 0.5))
	float YawLeftAmount{0.5f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0.5, ClampMax = 1))
	float YawRightAmount{0.5f};
};

USTRUCT(BlueprintType)
struct ALS_API FAlsSpineRotationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	uint8 bSpineRotationAllowed : 1{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InitialYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float CurrentYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};
};

UCLASS(Abstract, AutoExpandCategories = ("ALS|Settings"))
class ALS_API UAlsViewAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

public:
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	FAlsSpineRotationState SpineRotation; // using SpineRotation.YawAngle in UAlsAnimationInstance::GetControlRigInput 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient)
	FAlsLookState Look; // using YawAngle PitchAngle YawLeftAmount YawRightAmount YawForwardAmount in AnimBP

protected:
	UFUNCTION(BlueprintCallable, Category = "ALS|Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ReinitializeLook();

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void RefreshLook();

public:
	//virtual void NativeUpdateAnimation(float DeltaTime) override;

	//virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;

	void RefreshOnGameThread(float DeltaTime);

	void Refresh(float DeltaTime);
};
