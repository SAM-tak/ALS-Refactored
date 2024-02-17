// Fill out your copyright notice in the Description page of Project Settings.


#include "AlsPhysicalAnimationComponent.h"

#include "State/AlsPhysicalAnimationCurveState.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Curves/CurveFloat.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"
#include "AlsCharacter.h"
#include "AlsAnimationInstance.h"
#include "Engine/Canvas.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsPhysicalAnimationComponent)

float UAlsPhysicalAnimationComponent::GetLockedValue(const FAlsPhysicalAnimationCurveState& Curves, const FName& BoneName)
{
	if (BoneName == FName{TEXT("clavicle_l")})
	{
		return Curves.LockLeftArm;
	}
	if (BoneName == FName{TEXT("upperarm_l")})
	{
		return Curves.LockLeftArm;
	}
	if (BoneName == FName{TEXT("lowerarm_l")})
	{
		return Curves.LockLeftArm;
	}
	if (BoneName == FName{TEXT("hand_l")})
	{
		return Curves.LockLeftHand;
	}
	if (BoneName == FName{TEXT("clavicle_r")})
	{
		return Curves.LockRightArm;
	}
	if (BoneName == FName{TEXT("upperarm_r")})
	{
		return Curves.LockRightArm;
	}
	if (BoneName == FName{TEXT("lowerarm_r")})
	{
		return Curves.LockRightArm;
	}
	if (BoneName == FName{TEXT("hand_r")})
	{
		return Curves.LockRightHand;
	}
	if (BoneName == FName{TEXT("thigh_l")})
	{
		return Curves.LockLeftLeg;
	}
	if (BoneName == FName{TEXT("calf_l")})
	{
		return Curves.LockLeftLeg;
	}
	if (BoneName == UAlsConstants::FootLeftBoneName())
	{
		return Curves.LockLeftFoot;
	}
	if (BoneName == FName{TEXT("ball_l")})
	{
		return Curves.LockLeftFoot;
	}
	if (BoneName == FName{TEXT("thigh_r")})
	{
		return Curves.LockRightLeg;
	}
	if (BoneName == FName{TEXT("calf_r")})
	{
		return Curves.LockRightLeg;
	}
	if (BoneName == UAlsConstants::FootRightBoneName())
	{
		return Curves.LockRightFoot;
	}
	if (BoneName == FName{TEXT("ball_r")})
	{
		return Curves.LockRightFoot;
	}
	return 0.0f;
}

bool UAlsPhysicalAnimationComponent::IsProfileExist(const FName& ProfileName)
{
	for (auto i : GetSkeletalMesh()->Bodies)
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

bool UAlsPhysicalAnimationComponent::HasAnyProfile(const USkeletalBodySetup* BodySetup)
{
	for (const auto& ProfileName : CurrentProfileNames)
	{
		if (BodySetup->FindPhysicalAnimationProfile(ProfileName))
		{
			return true;
		}
	}
	return false;
}

bool UAlsPhysicalAnimationComponent::NeedsProfileChange(const AAlsCharacter* Character)
{
	bool RetVal = false;
	if (LocomotionAction != Character->GetLocomotionAction())
	{
		RetVal = true;
	}
	else if (LocomotionMode != Character->GetLocomotionMode())
	{
		RetVal = true;
	}
	else if (Stance != Character->GetStance())
	{
		RetVal = true;
	}
	else if (Gait != Character->GetGait())
	{
		RetVal = true;
	}
	else if (OverlayMode != Character->GetOverlayMode())
	{
		RetVal = true;
	}
	LocomotionAction = Character->GetLocomotionAction();
	LocomotionMode = Character->GetLocomotionMode();
	Stance = Character->GetStance();
	Gait = Character->GetGait();
	OverlayMode = Character->GetOverlayMode();
	if (CurrentOverrideMultiplyProfileNames != OverrideMultiplyProfileNames)
	{
		CurrentOverrideMultiplyProfileNames = OverrideMultiplyProfileNames;
		RetVal = true;
	}
	return RetVal;
}

void UAlsPhysicalAnimationComponent::ClearGameplayTags()
{
	LocomotionAction = FGameplayTag::EmptyTag;
	LocomotionMode = FGameplayTag::EmptyTag;
	Stance = FGameplayTag::EmptyTag;
	Gait = FGameplayTag::EmptyTag;
	OverlayMode = FGameplayTag::EmptyTag;
}

void UAlsPhysicalAnimationComponent::Refresh(float DeltaTime)
{
	USkeletalMeshComponent* Mesh{GetSkeletalMesh()};
	const FAlsPhysicalAnimationCurveState& Curves{Cast<UAlsAnimationInstance>(GetSkeletalMesh()->GetAnimInstance())->GetPhysicalAnimationCurveState()};

	bool bNeedUpdate = bActive;

	if (!bActive && CurrentProfileNames.Num() > 0)
	{
		for (auto BI : Mesh->Bodies)
		{
			if (USkeletalBodySetup* BodySetup = Cast<USkeletalBodySetup>(BI->BodySetup.Get()))
			{
				if (GetLockedValue(Curves, BodySetup->BoneName) <= 0.0f && HasAnyProfile(BodySetup))
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
				float LockedValue{GetLockedValue(Curves, BodySetup->BoneName)};
				if (!FAnimWeight::IsRelevant(LockedValue) && HasAnyProfile(BodySetup))
				{
					bActiveAny = true;
					if (BI->IsInstanceSimulatingPhysics())
					{
						float Speed = 1.0f / FMath::Max(0.000001f, BlendTimeOfBlendWeightOnActivate);
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
					if (FAnimWeight::IsRelevant(LockedValue))
					{
						if (BI->IsInstanceSimulatingPhysics())
						{
							BI->PhysicsBlendWeight = FMath::FInterpConstantTo(BI->PhysicsBlendWeight, 1.0f - LockedValue, DeltaTime, 15.0f);
						}
						else
						{
							if (!FAnimWeight::IsFullWeight(LockedValue))
							{
								BI->SetInstanceSimulatePhysics(true);
							}
							BI->PhysicsBlendWeight = 0.0f;
						}
					}
					else
					{
						float Speed = 1.0f / FMath::Max(0.000001f, BlendTimeOfBlendWeightOnDeactivate);
						BI->PhysicsBlendWeight = FMath::FInterpConstantTo(BI->PhysicsBlendWeight, 0.0f, DeltaTime, Speed);
					}
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

void UAlsPhysicalAnimationComponent::SelectProfile(const AAlsCharacter* Character)
{
	TArray<FName> NextProfileNames;
	TArray<FName> NextMultiplyProfileNames;

	auto ActionName = UAlsUtility::GetSimpleTagName(Character->GetLocomotionAction());
	auto ModeName = UAlsUtility::GetSimpleTagName(Character->GetLocomotionMode());
	auto StanceName = UAlsUtility::GetSimpleTagName(Character->GetStance());
	auto GaitName = UAlsUtility::GetSimpleTagName(Character->GetGait());
	auto OverlayModeName = UAlsUtility::GetSimpleTagName(Character->GetOverlayMode());
	auto ActionNameStr = ActionName.ToString();
	auto ModeNameStr = ActionName.ToString();
	auto StanceNameStr = StanceName.ToString();
	auto GaitNameStr = GaitName.ToString();
	auto OverlayModeNameStr = OverlayModeName.ToString();

	if (ActionName != NAME_None && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s:%s"), *ActionNameStr, *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ActionNameStr, *StanceNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ActionNameStr, *GaitNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ModeNameStr, *StanceNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s:%s"), *ModeNameStr, *GaitNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None && StanceName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ActionNameStr, *StanceNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None && GaitName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ActionNameStr, *GaitNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && StanceName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ModeNameStr, *StanceNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && GaitName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ModeNameStr, *GaitNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *ModeNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && StanceName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && GaitName != NAME_None && OverlayModeName != NAME_None)
	{
		auto ProfileName = FName{FString::Printf(TEXT("%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
		if (IsProfileExist(ProfileName))
		{
			NextProfileNames.Add(ProfileName);
		}
	}
	if (NextProfileNames.Num() == 0 && ActionName != NAME_None)
	{
		if (IsProfileExist(ActionName))
		{
			NextProfileNames.Add(ActionName);
		}
	}
	if (NextProfileNames.Num() == 0 && ModeName != NAME_None)
	{
		if (IsProfileExist(ModeName))
		{
			NextProfileNames.Add(ModeName);
		}
	}
	if (NextProfileNames.Num() == 0 && StanceName != NAME_None)
	{
		if (IsProfileExist(StanceName))
		{
			NextProfileNames.Add(StanceName);
		}
	}
	if (NextProfileNames.Num() == 0 && GaitName != NAME_None)
	{
		if (IsProfileExist(GaitName))
		{
			NextProfileNames.Add(GaitName);
		}
	}
	if (NextProfileNames.Num() == 0 && OverlayModeName != NAME_None)
	{
		if (IsProfileExist(OverlayModeName))
		{
			NextProfileNames.Add(OverlayModeName);
		}
	}
	if (NextProfileNames.Num() == 0 && IsProfileExist(UAlsConstants::DefaultPAProfileName()))
	{
		NextProfileNames.Add(UAlsConstants::DefaultPAProfileName());
	}

	// add additional profiles if exists.

	if (NextProfileNames.Num() > 0)
	{
		if (ActionName != NAME_None && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s:%s"), *ActionNameStr, *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ActionNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ActionNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ModeNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s:%s"), *ModeNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ActionNameStr, *StanceNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ActionNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ActionNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ModeNameStr, *StanceNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ModeNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *ModeNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s"), *ActionNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s"), *ModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s"), *StanceNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s"), *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("+%s"), *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextProfileNames.Add(ProfileName);
			}
		}
	}

	if (NextProfileNames.Num() > 0)
	{
		// Process multiply profiles if exists.

		if (ActionName != NAME_None && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s:%s"), *ActionNameStr, *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s"), *ActionNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ActionNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ActionNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *StanceNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s:%s"), *ModeNameStr, *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ActionNameStr, *StanceNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ActionNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ActionNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ModeNameStr, *StanceNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ModeNameStr, *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *ModeNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && StanceName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *StanceNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && GaitName != NAME_None && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s:%s"), *GaitNameStr, *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (ActionName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s"), *ActionNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && ModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s"), *ModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && StanceName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s"), *StanceNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && GaitName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s"), *GaitNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}
		if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) && OverlayModeName != NAME_None)
		{
			auto ProfileName = FName{FString::Printf(TEXT("*%s"), *OverlayModeNameStr)};
			if (IsProfileExist(ProfileName))
			{
				NextMultiplyProfileNames.Add(ProfileName);
			}
		}

		NextMultiplyProfileNames.Append(MultiplyProfileNames);
	}

	if (NextProfileNames != CurrentProfileNames || NextMultiplyProfileNames != CurrentMultiplyProfileNames)
	{
		bool First = true;
		for (const auto& NextProfileName : NextProfileNames)
		{
			ApplyPhysicalAnimationProfileBelow(NAME_None, NextProfileName);
			GetSkeletalMesh()->SetConstraintProfileForAll(NextProfileName, First);
			First = false;
		}
		Activate();
		CurrentProfileNames = NextProfileNames;

		for (const auto& NextMultiplyProfileName : NextMultiplyProfileNames)
		{
			ApplyPhysicalAnimationProfileBelow(NAME_None, NextMultiplyProfileName);
			GetSkeletalMesh()->SetConstraintProfileForAll(NextMultiplyProfileName);
		}
		CurrentMultiplyProfileNames = NextMultiplyProfileNames;
	}
}

void UAlsPhysicalAnimationComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// TODO : Cache
	auto* Character{Cast<AAlsCharacter>(GetSkeletalMesh()->GetOwner())};

	// Apply special behaviour when changed Ragdolling state

	if (Character->GetLocomotionAction() == AlsLocomotionActionTags::Ragdolling)
	{
		if (!bRagdolling)
		{
			bRagdolling = true;
			GetSkeletalMesh()->SetAllBodiesBelowSimulatePhysics(UAlsConstants::PelvisBoneName(), true);
			GetSkeletalMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
			ApplyPhysicalAnimationProfileBelow(NAME_None, NAME_None, true, true);
		}
	}
	else
	{
		if (bRagdolling)
		{
			bRagdolling = false;
			CurrentProfileNames.Reset();
			CurrentMultiplyProfileNames.Reset();
			ClearGameplayTags();
			GetSkeletalMesh()->SetAllBodiesSimulatePhysics(false);
			GetSkeletalMesh()->SetAllBodiesPhysicsBlendWeight(0.0f);
			GetSkeletalMesh()->SetConstraintProfileForAll(NAME_None, true);

			if (bActive)
			{
				GetSkeletalMesh()->SetCollisionObjectType(PrevCollisionObjectType);
				GetSkeletalMesh()->SetCollisionEnabled(PrevCollisionEnabled);
				bActive = false;
			}

			// skip to next frame

			Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
			return;
		}
	}

	// Choose Physical Animation Profile

	if (OverrideProfileNames.Num() > 0)
	{
		if (CurrentProfileNames != OverrideProfileNames || CurrentMultiplyProfileNames != MultiplyProfileNames)
		{
			bool first = true;
			for (const auto& CurrentProfileName : OverrideProfileNames)
			{
				ApplyPhysicalAnimationProfileBelow(NAME_None, CurrentProfileName);
				GetSkeletalMesh()->SetConstraintProfileForAll(CurrentProfileName, first);
				first = false;
			}
			Activate();
			CurrentProfileNames = OverrideProfileNames;
			ClearGameplayTags();

			for (const auto& MultiplyProfileName : MultiplyProfileNames)
			{
				ApplyPhysicalAnimationProfileBelow(NAME_None, MultiplyProfileName);
				GetSkeletalMesh()->SetConstraintProfileForAll(MultiplyProfileName);
			}
			CurrentMultiplyProfileNames = MultiplyProfileNames;
		}
	}
	else if(NeedsProfileChange(Character))
	{
		SelectProfile(Character);
	}

	// Update PhysicsBlendWeight and Collision settings

	if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Ragdolling) || !Character->GetRagdollingState().bFreezing)
	{
		Refresh(DeltaTime);
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAlsPhysicalAnimationComponent::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& HorizontalLocation, float& VerticalLocation)
{
	const auto Scale{FMath::Min(Canvas->SizeX / (1280.0f * Canvas->GetDPIScale()), Canvas->SizeY / (720.0f * Canvas->GetDPIScale()))};

	VerticalLocation += 4.0f * Scale;

	FCanvasTextItem Text{
		FVector2D::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetMediumFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale * 0.75f, Scale * 0.75f};
	Text.EnableShadow(FLinearColor::Black);

	const auto RowOffset{12.0f * Scale};
	const auto ColumnOffset{145.0f * Scale};
	
	TStringBuilder<256> DebugStringBuilder;

	for (const auto& ProfileName : CurrentProfileNames)
	{
		DebugStringBuilder.Appendf(TEXT("%s "), *ProfileName.ToString());
	}

	for (const auto& ProfileName : CurrentMultiplyProfileNames)
	{
		DebugStringBuilder.Appendf(TEXT("%s "), *ProfileName.ToString());
	}

	Text.Text = FText::AsCultureInvariant(DebugStringBuilder.ToString());
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;

	for (auto BI : GetSkeletalMesh()->Bodies)
	{
		Text.SetColor(FMath::Lerp(FLinearColor::Gray, FLinearColor::Red, UAlsMath::Clamp01(BI->PhysicsBlendWeight)));

		Text.Text = FText::AsCultureInvariant(FString::Printf(TEXT("%s %s %1.2f"), *BI->GetBodySetup()->BoneName.ToString(),
			BI->IsInstanceSimulatingPhysics() ? TEXT("ON") : TEXT("OFF"),
			BI->PhysicsBlendWeight));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

		VerticalLocation += RowOffset;
	}
}
