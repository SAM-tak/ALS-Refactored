#include "State/AlsPhysicalAnimationState.h"

#include "State/AlsPhysicalAnimationCurveState.h"
#include "Settings/AlsPhysicalAnimationSettings.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Curves/CurveFloat.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsPhysicalAnimationState)

namespace
{
	bool IsLocked(const FAlsPhysicalAnimationCurveState& Curves, const FName& BoneName)
	{
		if (BoneName == FName{TEXT("clavicle_l")})
		{
			return Curves.LockLeftArm > 0.0f;
		}
		if (BoneName == FName{TEXT("upperarm_l")})
		{
			return Curves.LockLeftArm > 0.0f;
		}
		if (BoneName == FName{TEXT("lowerarm_l")})
		{
			return Curves.LockLeftArm > 0.0f;
		}
		if (BoneName == FName{TEXT("hand_l")})
		{
			return Curves.LockLeftHand > 0.0f;
		}
		if (BoneName == FName{TEXT("clavicle_r")})
		{
			return Curves.LockRightArm > 0.0f;
		}
		if (BoneName == FName{TEXT("upperarm_r")})
		{
			return Curves.LockRightArm > 0.0f;
		}
		if (BoneName == FName{TEXT("lowerarm_r")})
		{
			return Curves.LockRightArm > 0.0f;
		}
		if (BoneName == FName{TEXT("hand_r")})
		{
			return Curves.LockRightHand > 0.0f;
		}
		if (BoneName == FName{TEXT("thigh_l")})
		{
			return Curves.LockLeftLeg > 0.0f;
		}
		if (BoneName == FName{TEXT("calf_l")})
		{
			return Curves.LockLeftLeg > 0.0f;
		}
		if (BoneName == UAlsConstants::FootLeftBoneName())
		{
			return Curves.LockLeftFoot > 0.0f;
		}
		if (BoneName == FName{TEXT("ball_l")})
		{
			return Curves.LockLeftFoot > 0.0f;
		}
		if (BoneName == FName{TEXT("thigh_r")})
		{
			return Curves.LockRightLeg > 0.0f;
		}
		if (BoneName == FName{TEXT("calf_r")})
		{
			return Curves.LockRightLeg > 0.0f;
		}
		if (BoneName == UAlsConstants::FootRightBoneName())
		{
			return Curves.LockRightFoot > 0.0f;
		}
		if (BoneName == FName{TEXT("ball_r")})
		{
			return Curves.LockRightFoot > 0.0f;
		}
		return false;
	}
}

bool FAlsPhysicalAnimationState::HasAnyProfile(const USkeletalBodySetup* BodySetup)
{
	for (const auto& ProfileName : ProfileNames)
	{
		if (BodySetup->FindPhysicalAnimationProfile(ProfileName))
		{
			return true;
		}
	}
	return false;
}

bool FAlsPhysicalAnimationState::NeedsProfielChange(const FGameplayTag& NewLocomotionMode, const FGameplayTag& NewLocomotionAction,
	const FGameplayTag& NewStance, const FGameplayTag& NewGait, const FGameplayTag& NewOverlayMode, const TArray<FName>& NewMultiplyProfileNames)
{
	bool RetVal = false;
	if (LocomotionAction != NewLocomotionAction)
	{
		RetVal = true;
	}
	else if(LocomotionMode != NewLocomotionMode)
	{
		RetVal = true;
	}
	else if (Stance != NewStance)
	{
		RetVal = true;
	}
	else if (Gait != NewGait)
	{
		RetVal = true;
	}
	else if (OverlayMode != NewOverlayMode)
	{
		RetVal = true;
	}
	LocomotionAction = NewLocomotionAction;
	LocomotionMode = NewLocomotionMode;
	Stance = NewStance;
	Gait = NewGait;
	OverlayMode = NewOverlayMode;
	if (MultiplyProfileNames != NewMultiplyProfileNames)
	{
		MultiplyProfileNames = NewMultiplyProfileNames;
		RetVal = true;
	}
	return RetVal;
}

void FAlsPhysicalAnimationState::ClearAlsTags()
{
	LocomotionAction = FGameplayTag::EmptyTag;
	LocomotionMode = FGameplayTag::EmptyTag;
	Stance = FGameplayTag::EmptyTag;
	Gait = FGameplayTag::EmptyTag;
	OverlayMode = FGameplayTag::EmptyTag;
}

void FAlsPhysicalAnimationState::Refresh(float DeltaTime, USkeletalMeshComponent* Mesh, const FAlsPhysicalAnimationSettings& Settings, const FAlsPhysicalAnimationCurveState& Curves)
{
	bool bNeedUpdate = bActive;

	if (!bActive && ProfileNames.Num() > 0)
	{
		for (auto BI : Mesh->Bodies)
		{
			if (USkeletalBodySetup* BodySetup = Cast<USkeletalBodySetup>(BI->BodySetup.Get()))
			{
				if (!IsLocked(Curves, BodySetup->BoneName) && HasAnyProfile(BodySetup))
				{
					bNeedUpdate = true;
					break;
				}
			}
		}
	}

	bool bActiveAny = false;

	if (bNeedUpdate)
	{
		for (auto BI : Mesh->Bodies)
		{
			if (USkeletalBodySetup* BodySetup = Cast<USkeletalBodySetup>(BI->BodySetup.Get()))
			{
				if (!IsLocked(Curves, BodySetup->BoneName) && HasAnyProfile(BodySetup))
				{
					bActiveAny = true;
					float Speed = 1.0f / FMath::Max(0.000001f, Settings.BlendTimeOfBlendWeightOnActivate);
					if (BI->IsInstanceSimulatingPhysics())
					{
						BI->PhysicsBlendWeight = FMath::Min(1.0f, FMath::FInterpConstantTo(BI->PhysicsBlendWeight, 1.0f, DeltaTime, Speed));
					}
					else
					{
						BI->SetInstanceSimulatePhysics(true);
						BI->PhysicsBlendWeight = 0.0f;
					}
				}
				else
				{
					float Speed = 1.0f / FMath::Max(0.000001f, Settings.BlendTimeOfBlendWeightOnDeactivate);
					BI->PhysicsBlendWeight = FMath::Max(0.0f, FMath::FInterpConstantTo(BI->PhysicsBlendWeight, 0.0f, DeltaTime, Speed));
					if (BI->PhysicsBlendWeight == 0.0f)
					{
						if (BI->IsInstanceSimulatingPhysics())
						{
							BI->SetInstanceSimulatePhysics(false);
						}
					}
					else
					{
						bActiveAny = true;
					}
				}
			}
		}

		// Trick. Calling a no effect method To call skeletal mesh's private method "UpdateEndPhysicsTickRegisteredState" and "UpdateClothTickRegisteredState".
		Mesh->AccumulateAllBodiesBelowPhysicsBlendWeight(NAME_None, 0.0f);
	}
	
	if (bActiveAny && !bActive)
	{
		PrevCollisionObjectType = TEnumAsByte(Mesh->GetCollisionObjectType());
		PrevCollisionEnabled = TEnumAsByte(Mesh->GetCollisionEnabled());
		Mesh->SetCollisionObjectType(ECC_PhysicsBody);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		bActive = true;
	}

	if (!bActiveAny && bActive)
	{
		Mesh->SetCollisionObjectType(PrevCollisionObjectType);
		Mesh->SetCollisionEnabled(PrevCollisionEnabled);
		bActive = false;
	}
}
