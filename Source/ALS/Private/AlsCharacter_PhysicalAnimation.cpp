#include "AlsCharacter.h"

#include "AlsAnimationInstance.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

namespace
{
	bool IsProfileExist(const USkeletalMeshComponent* Mesh, const FName& ProfileName)
	{
		for (auto i : Mesh->Bodies)
		{
			if (USkeletalBodySetup* BodySetup = Cast<USkeletalBodySetup>(i->BodySetup.Get()))
			{
				if (BodySetup->FindPhysicalAnimationProfile(ProfileName))
				{
					return true;
				}
			}
		}
		return false;
	}
}

FName AAlsCharacter::PhysicalAnimationComponentName(TEXT("PhysicalAnimComp"));

void AAlsCharacter::RefreshPhysicalAnimation(float DeltaTime)
{
	// Choose Physical Animation Profile and active part

	bool bFound = OverridePAProfileName != NAME_None && IsProfileExist(GetMesh(), OverridePAProfileName);

	FName CurrentProfileName = bFound ? OverridePAProfileName : NAME_None;

	if (LocomotionAction == AlsLocomotionActionTags::Ragdolling)
	{
		if (!PhysicalAnimationState.bRagdolling)
		{
			PhysicalAnimationState.bRagdolling = true;
			GetMesh()->SetAllBodiesBelowSimulatePhysics(UAlsConstants::PelvisBoneName(), true);
			GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
			PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, NAME_None, true, true);
		}
	}
	else
	{
		if (PhysicalAnimationState.bRagdolling)
		{
			PhysicalAnimationState.bRagdolling = false;
			PhysicalAnimationState.ProfileName = NAME_None;
			GetMesh()->SetAllBodiesSimulatePhysics(false);
			GetMesh()->SetAllBodiesPhysicsBlendWeight(0.0f);
			GetMesh()->SetConstraintProfileForAll(NAME_None, true);

			if (PhysicalAnimationState.bActive)
			{
				GetMesh()->SetCollisionObjectType(PhysicalAnimationState.PrevCollisionObjectType);
				GetMesh()->SetCollisionEnabled(PhysicalAnimationState.PrevCollisionEnabled);
				PhysicalAnimationState.bActive = false;
			}

			return; // skip to next frame
		}
	}

	if (!bFound)
	{
		auto LocomotionName = LocomotionAction.IsValid() ? UAlsUtility::GetSimpleTagName(LocomotionAction) : UAlsUtility::GetSimpleTagName(LocomotionMode);
		auto StanceName = UAlsUtility::GetSimpleTagName(Stance);
		auto GaitName = UAlsUtility::GetSimpleTagName(Gait);
		auto OverlayModeName = UAlsUtility::GetSimpleTagName(OverlayMode);
		auto LocomotionNameStr = LocomotionName.ToString();
		auto StanceNameStr = StanceName.ToString();
		auto GaitNameStr = GaitName.ToString();
		auto OverlayModeNameStr = OverlayModeName.ToString();

		if (!bFound && LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *LocomotionNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *LocomotionNameStr, *StanceNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *LocomotionNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *LocomotionNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileName = ProfileName;
				bFound = true;
			}
		}
		if (!bFound && LocomotionName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), LocomotionName))
			{
				CurrentProfileName = LocomotionName;
				bFound = true;
			}
		}
		if (!bFound && StanceName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), StanceName))
			{
				CurrentProfileName = StanceName;
				bFound = true;
			}
		}
		if (!bFound && GaitName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), GaitName))
			{
				CurrentProfileName = GaitName;
			}
		}
		if (!bFound && OverlayModeName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), OverlayModeName))
			{
				CurrentProfileName = OverlayModeName;
				bFound = true;
			}
		}
		if (!bFound && IsProfileExist(GetMesh(), UAlsConstants::DefaultPAProfileName()))
		{
			CurrentProfileName = UAlsConstants::DefaultPAProfileName();
		}
	}

	if (PhysicalAnimationState.ProfileName != CurrentProfileName)
	{
		PhysicalAnimationState.ProfileName = CurrentProfileName;
		PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, CurrentProfileName);
		PhysicalAnimation->Activate();
		if (Settings->PhysicalAnimation.bUseConstraintProfile)
		{
			GetMesh()->SetConstraintProfileForAll(CurrentProfileName, true);
		}
	}

	// Update PhysicsBlendWeight and Collision settings

	if (LocomotionAction != AlsLocomotionActionTags::Ragdolling || !RagdollingState.bFreezing)
	{
		PhysicalAnimationState.Refresh(DeltaTime, GetMesh(), Settings->PhysicalAnimation, AnimationInstance->GetPhysicalAnimationCurveState());
	}
}
