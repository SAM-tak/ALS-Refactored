#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "AlsLayeringAnimInstance.generated.h"

UCLASS(Abstract, AutoExpandCategories = ("ALS|Settings"))
class ALS_API UAlsLayeringAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float HeadBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float HeadAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float HeadSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftLocalSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftMeshSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightLocalSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightMeshSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float HandLeftBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float HandRightBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float SpineBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float SpineAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float SpineSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float PelvisBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float PelvisSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float LegsBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|State", Transient, Meta = (ClampMin = 0, ClampMax = 1))
	float LegsSlotBlendAmount{1.0f};

public:
	void Refresh();
};
