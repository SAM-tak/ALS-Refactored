#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationState.generated.h"

enum class EAlsPhysicalAnimationPartMask : uint16
{
	None,
	Pelvis    = 1 << 0,
	Torso     = 1 << 1,
	Head      = 1 << 2,
	LeftArm   = 1 << 3,
	RightArm  = 1 << 4,
	LeftLeg   = 1 << 5,
	RightLeg  = 1 << 6,
	LeftHand  = 1 << 7,
	RightHand = 1 << 8,
	LeftFoot  = 1 << 9,
	RightFoot = 1 << 10,
	WholeBody = Pelvis | Torso | Head | LeftArm | RightArm | LeftLeg | RightLeg | LeftHand | RightHand | LeftFoot | RightFoot,
	BelowTorso = Torso | Head | LeftArm | RightArm | LeftHand | RightHand,
	BelowLeftArm = LeftArm | LeftHand,
	BelowRightArm = RightArm | RightHand,
	BelowLeftLeg = LeftLeg | LeftFoot,
	BelowRightLeg = RightLeg | RightFoot,
};
ENUM_CLASS_FLAGS(EAlsPhysicalAnimationPartMask)

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	float BlendWeight{0.0f};

	//static const int PartNum = 11;
	//float PartBlendWeight[PartNum]{0.0f,};

	uint16 ActiveParts{0};

	bool bIsRagdolling{false};

	bool bInitialized{false};
};
