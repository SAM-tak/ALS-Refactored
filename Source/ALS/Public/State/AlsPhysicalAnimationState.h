#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationState.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAlsPhysicalAnimationPartMask : uint8
{
	None,
	WholeBody = 1 << 0,
	Torso     = 1 << 1,
	LeftArm   = 1 << 2,
	RightArm  = 1 << 3,
	LeftLeg   = 1 << 4,
	RightLeg  = 1 << 5,
};
ENUM_CLASS_FLAGS(EAlsPhysicalAnimationPartMask)

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float BlendWeight{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float PrevBlendWeight{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", meta = (Bitmask, BitmaskEnum = "/Script/ALS.EAlsPhysicalAnimationPartMask"))
	uint8 ActiveParts{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", meta = (Bitmask, BitmaskEnum = "/Script/ALS.EAlsPhysicalAnimationPartMask"))
	uint8 PrevActiveParts{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};
};
