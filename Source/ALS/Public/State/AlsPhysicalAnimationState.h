#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationState.generated.h"

UENUM(BlueprintType)
enum EAlsPhysicalAnimationPart
{
	Pelvis,		   // 0
	Torso,		   // 1
	LeftLeg,	   // 2
	RightLeg,	   // 3
	Neck,		   // 4
	LeftClavicle,  // 5
	RightClavicle, // 6
	LeftFoot,	   // 7
	RightFoot,	   // 8
	Head,		   // 9
	LeftArm,	   // 10
	RightArm,	   // 11
	LeftHand,	   // 12
	RightHand,	   // 13
	MAX_NUM		   // 14
};

enum class EAlsPhysicalAnimationPartMask : uint16
{
	None,
	Pelvis        = 1 << EAlsPhysicalAnimationPart::Pelvis,
	Torso         = 1 << EAlsPhysicalAnimationPart::Torso,
	LeftLeg       = 1 << EAlsPhysicalAnimationPart::LeftLeg,
	RightLeg      = 1 << EAlsPhysicalAnimationPart::RightLeg,
	Neck          = 1 << EAlsPhysicalAnimationPart::Neck,
	LeftClavicle  = 1 << EAlsPhysicalAnimationPart::LeftClavicle,
	RightClavicle = 1 << EAlsPhysicalAnimationPart::RightClavicle,
	LeftFoot      = 1 << EAlsPhysicalAnimationPart::LeftFoot,
	RightFoot     = 1 << EAlsPhysicalAnimationPart::RightFoot,
	LeftArm       = 1 << EAlsPhysicalAnimationPart::LeftArm,
	RightArm      = 1 << EAlsPhysicalAnimationPart::RightArm,
	Head          = 1 << EAlsPhysicalAnimationPart::Head,
	LeftHand      = 1 << EAlsPhysicalAnimationPart::LeftHand,
	RightHand     = 1 << EAlsPhysicalAnimationPart::RightHand,
	WholeBody     = Pelvis | Torso | LeftLeg | RightLeg | Neck | LeftClavicle | RightClavicle
	              | LeftFoot | RightFoot | LeftArm | RightArm | Head | LeftHand | RightHand,
	BelowTorso    = Torso | Neck | LeftClavicle | RightClavicle | Head | LeftArm | RightArm | LeftHand | RightHand,
	BelowNeck     = Neck | Head,
	BelowLeftArm  = LeftArm | LeftHand,
	BelowRightArm = RightArm | RightHand,
	BelowLeftLeg  = LeftLeg | LeftFoot,
	BelowRightLeg = RightLeg | RightFoot,
};
ENUM_CLASS_FLAGS(EAlsPhysicalAnimationPartMask)

UENUM(BlueprintType)
enum class EAlsPhysicalAnimationProfile : uint8
{
	None,
	Default,
	Ragdoll,
	Injured,
};

USTRUCT(BlueprintType)
struct ALS_API FAlsPhysicalAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> PrevCollisionObjectType{ECC_Pawn};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionEnabled::Type> PrevCollisionEnabled{ECollisionEnabled::QueryOnly};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	EAlsPhysicalAnimationProfile Current{0};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS")
	bool bInitialized{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS")
	bool bActive{false};

	float PartBlendWeight[EAlsPhysicalAnimationPart::MAX_NUM]{};

	uint16 ActiveParts{0};
};
