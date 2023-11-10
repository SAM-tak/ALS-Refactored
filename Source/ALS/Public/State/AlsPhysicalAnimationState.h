#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationState.generated.h"

class USkeletalMeshComponent;
struct FAlsPhysicalAnimationCurveState;
struct FAlsPhysicalAnimationSettings;

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS")
	FName ProfileName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS")
	bool bActive{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS")
	bool bRagdolling{ false };

	void Refresh(float DelaTime, USkeletalMeshComponent* Mesh, const FAlsPhysicalAnimationSettings& Settings, const FAlsPhysicalAnimationCurveState& Curves);
};
