#pragma once

#include "Utility/AlsMath.h"
#include "AlsPhysicalAnimationState.generated.h"

UENUM(BlueprintType)
enum EAlsPhysicalAnimationPart
{
	Pelvis,		// 0
	Torso,		// 1
	LeftLeg,	// 2
	RightLeg,	// 3
	Head,		// 4
	LeftArm,	// 5
	RightArm,	// 6
	LeftFoot,	// 7
	RightFoot,	// 8
	LeftHand,	// 9
	RightHand,	// 10
	MAX_NUM		// 11
};

enum class EAlsPhysicalAnimationPartMask : uint16
{
	None,
	Pelvis    = 1 << EAlsPhysicalAnimationPart::Pelvis,
	Torso     = 1 << EAlsPhysicalAnimationPart::Torso,
	LeftLeg   = 1 << EAlsPhysicalAnimationPart::LeftLeg,
	RightLeg  = 1 << EAlsPhysicalAnimationPart::RightLeg,
	Head      = 1 << EAlsPhysicalAnimationPart::Head,
	LeftArm   = 1 << EAlsPhysicalAnimationPart::LeftArm,
	RightArm  = 1 << EAlsPhysicalAnimationPart::RightArm,
	LeftFoot  = 1 << EAlsPhysicalAnimationPart::LeftFoot,
	RightFoot = 1 << EAlsPhysicalAnimationPart::RightFoot,
	LeftHand  = 1 << EAlsPhysicalAnimationPart::LeftHand,
	RightHand = 1 << EAlsPhysicalAnimationPart::RightHand,
	WholeBody = Pelvis | Torso | Head | LeftArm | RightArm | LeftLeg | RightLeg | LeftHand | RightHand | LeftFoot | RightFoot,
	BelowTorso = Torso | Head | LeftArm | RightArm | LeftHand | RightHand,
	BelowLeftArm = LeftArm | LeftHand,
	BelowRightArm = RightArm | RightHand,
	BelowLeftLeg = LeftLeg | LeftFoot,
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
