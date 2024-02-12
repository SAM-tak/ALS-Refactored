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
		TArray<FName> CurrentMultiplyProfileNames;

		auto ActionName = UAlsUtility::GetSimpleTagName(LocomotionAction);
		auto ModeName = UAlsUtility::GetSimpleTagName(LocomotionMode);
		auto StanceName = UAlsUtility::GetSimpleTagName(Stance);
		auto GaitName = UAlsUtility::GetSimpleTagName(Gait);
		auto OverlayModeName = UAlsUtility::GetSimpleTagName(OverlayMode);
		auto ActionNameStr = ActionName.ToString();
		auto ModeNameStr = ActionName.ToString();
		auto StanceNameStr = StanceName.ToString();
		auto GaitNameStr = GaitName.ToString();
		auto OverlayModeNameStr = OverlayModeName.ToString();

		if (ActionName != NAME_None && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s:%s"), *ActionNameStr, *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ActionNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ActionNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ModeNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ModeNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ActionNameStr, *StanceNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ActionNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ModeNameStr, *StanceNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ModeNameStr, *GaitNameStr)};
			if (IsProfileExist(GetMesh(), ProfileName))
			{
				CurrentProfileNames.Add(ProfileName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ModeNameStr, *OverlayModeNameStr)};
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
		if (CurrentProfileNames.Num() == 0 && ActionName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), ActionName))
			{
				CurrentProfileNames.Add(ActionName);
			}
		}
		if (CurrentProfileNames.Num() == 0 && ModeName != NAME_None)
		{
			if (IsProfileExist(GetMesh(), ModeName))
			{
				CurrentProfileNames.Add(ModeName);
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
			if (ActionName != NAME_None && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s:%s"), *ActionNameStr, *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ActionNameStr, *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ActionNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ModeNameStr, *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ModeNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ActionNameStr, *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ActionNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ActionNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ModeNameStr, *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ModeNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ModeNameStr, *OverlayModeNameStr)};
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
			if (ActionName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s"), *ActionNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("+%s"), *ModeNameStr)};
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

		if (CurrentProfileNames.Num() > 0)
		{
			// Process multiply profiles if exists.

			if (ActionName != NAME_None && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s:%s"), *ActionNameStr, *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ActionNameStr, *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ActionNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ActionNameStr, *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ActionNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ActionNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ModeNameStr, *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ModeNameStr, *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ModeNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && StanceName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && GaitName != NAME_None && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (ActionName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *ActionNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && ModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *ModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && StanceName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *StanceNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && GaitName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *GaitNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}
			if (LocomotionAction != AlsLocomotionActionTags::Ragdolling && OverlayModeName != NAME_None)
			{
				auto ProfileName = FName{FString::Printf(TEXT("*%s"), *OverlayModeNameStr)};
				if (IsProfileExist(GetMesh(), ProfileName))
				{
					CurrentMultiplyProfileNames.Add(ProfileName);
				}
			}

			CurrentMultiplyProfileNames.Append(MultiplyPAProfileNames);
		}

		if (PhysicalAnimationState.ProfileNames != CurrentProfileNames || PhysicalAnimationState.MultiplyProfileNames != CurrentMultiplyProfileNames)
		{
			bool First = true;
			for (const auto& CurrentProfileName : CurrentProfileNames)
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, CurrentProfileName);
				GetMesh()->SetConstraintProfileForAll(CurrentProfileName, First);
				First = false;
			}
			PhysicalAnimation->Activate();
			PhysicalAnimationState.ProfileNames = CurrentProfileNames;

			for (const auto& MultiplyProfileName : CurrentMultiplyProfileNames)
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, MultiplyProfileName);
				GetMesh()->SetConstraintProfileForAll(MultiplyProfileName);
			}
			PhysicalAnimationState.MultiplyProfileNames = CurrentMultiplyProfileNames;
		}
	}

	// Update PhysicsBlendWeight and Collision settings

	if (LocomotionAction != AlsLocomotionActionTags::Ragdolling || !RagdollingState.bFreezing)
	{
		PhysicalAnimationState.Refresh(DeltaTime, GetMesh(), Settings->PhysicalAnimation, AnimationInstance->GetPhysicalAnimationCurveState());
	}
}
