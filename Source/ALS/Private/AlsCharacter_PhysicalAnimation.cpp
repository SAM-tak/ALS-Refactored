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
	// Apply special behaviour when changed Ragdolling state

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
			PhysicalAnimationState.ProfileNames.Reset();
			PhysicalAnimationState.MultiplyProfileNames.Reset();
			PhysicalAnimationState.ClearAlsTags();
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

	// Choose Physical Animation Profile

	if (OverridePAProfileNames.Num() > 0)
	{
		if (PhysicalAnimationState.ProfileNames != OverridePAProfileNames)
		{
			bool first = true;
			for (const auto& CurrentProfileName : OverridePAProfileNames)
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, CurrentProfileName);
				GetMesh()->SetConstraintProfileForAll(CurrentProfileName, first);
				first = false;
			}
			PhysicalAnimation->Activate();
			PhysicalAnimationState.ProfileNames = OverridePAProfileNames;
			PhysicalAnimationState.ClearAlsTags();
		}
		if (PhysicalAnimationState.MultiplyProfileNames != MultiplyPAProfileNames)
		{
			for (const auto& MultiplyProfileName : MultiplyPAProfileNames)
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, MultiplyProfileName);
				GetMesh()->SetConstraintProfileForAll(MultiplyProfileName);
			}
			PhysicalAnimationState.MultiplyProfileNames = MultiplyPAProfileNames;
		}
	}
	else if(PhysicalAnimationState.NeedsProfielChange(LocomotionAction, LocomotionMode, Stance, Gait, OverlayMode, MultiplyPAProfileNames))
	{
		TArray<FName> CurrentProfileNames;

		auto LocomotionName = LocomotionAction.IsValid() ? UAlsUtility::GetSimpleTagName(LocomotionAction) : UAlsUtility::GetSimpleTagName(LocomotionMode);
		auto StanceName = UAlsUtility::GetSimpleTagName(Stance);
		auto GaitName = UAlsUtility::GetSimpleTagName(Gait);
		auto OverlayModeName = UAlsUtility::GetSimpleTagName(OverlayMode);
		auto LocomotionNameStr = LocomotionName.ToString();
		auto StanceNameStr = StanceName.ToString();
		auto GaitNameStr = GaitName.ToString();
		auto OverlayModeNameStr = OverlayModeName.ToString();

		if (LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *LocomotionNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *LocomotionNameStr, *StanceNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *LocomotionNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *LocomotionNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && LocomotionName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), LocomotionName))
			{
				CurrentProfileNames.Add(LocomotionName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && StanceName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), StanceName))
			{
				CurrentProfileNames.Add(StanceName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && GaitName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), GaitName))
			{
				CurrentProfileNames.Add(GaitName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && OverlayModeName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), OverlayModeName))
			{
				CurrentProfileNames.Add(OverlayModeName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && IsProfileExist(GetMesh(), UAlsConstants::DefaultPAProfileName()))
		{
			CurrentProfileNames.Add(UAlsConstants::DefaultPAProfileName());
		}

		// add additional profiles if exists.

		if (CurrentProfileNames.Num() > 0)
		{
			if (LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *LocomotionNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *LocomotionNameStr, *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *LocomotionNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *LocomotionNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && LocomotionName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s"), *LocomotionNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s"), *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s"), *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s"), *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
		}

		if (PhysicalAnimationState.ProfileNames != CurrentProfileNames)
		{
			bool first = true;
			for (const auto& CurrentProfileName : CurrentProfileNames)
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, CurrentProfileName);
				GetMesh()->SetConstraintProfileForAll(CurrentProfileName, first);
				first = false;
			}
			PhysicalAnimation->Activate();
			PhysicalAnimationState.ProfileNames = CurrentProfileNames;
		}

		if (CurrentProfileNames.Num() > 0)
		{
			// Process multiply profiles if exists.

			TArray<FName> MultiplyProfileNames;

			if (LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *LocomotionNameStr, *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *LocomotionNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *LocomotionNameStr, *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *LocomotionNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *LocomotionNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *LocomotionNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					MultiplyProfileNames.Add(ProfileName);
				}
			}

			MultiplyProfileNames.Append(MultiplyPAProfileNames);

			for (const auto& MultiplyProfileName : MultiplyProfileNames)
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, MultiplyProfileName);
				GetMesh()->SetConstraintProfileForAll(MultiplyProfileName);
			}
		}
	}

	// Update PhysicsBlendWeight and Collision settings

	if (LocomotionAction != AlsLocomotionActionTags::Ragdolling || !RagdollingState.bFreezing)
	{
		PhysicalAnimationState.Refresh(DeltaTime, GetMesh(), Settings->PhysicalAnimation, AnimationInstance->GetPhysicalAnimationCurveState());
	}
}
