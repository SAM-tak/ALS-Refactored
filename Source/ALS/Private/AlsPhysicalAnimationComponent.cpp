// Fill out your copyright notice in the Description page of Project Settings.


#include "AlsPhysicalAnimationComponent.h"

#include "AlsCharacter.h"
#include "AlsAnimationInstance.h"
#include "State/AlsPhysicalAnimationCurveState.h"
#include "Abilities/AlsGameplayAbility_Ragdolling.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Curves/CurveFloat.h"
#include "Engine/Canvas.h"
#include "AbilitySystemComponent.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsPhysicalAnimationComponent)

UAlsPhysicalAnimationComponent::UAlsPhysicalAnimationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UAlsPhysicalAnimationComponent::IsProfileExist(const FName& ProfileName) const
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

bool UAlsPhysicalAnimationComponent::HasAnyProfile(const USkeletalBodySetup* BodySetup) const
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

bool UAlsPhysicalAnimationComponent::NeedsProfileChange()
{
	bool RetVal = CurrentGameplayTags != PreviousGameplayTags;
	PreviousGameplayTags = CurrentGameplayTags;
	if (CurrentMultiplyProfileNames != MultiplyProfileNames)
	{
		CurrentMultiplyProfileNames = MultiplyProfileNames;
		RetVal = true;
	}
	return RetVal;
}

void UAlsPhysicalAnimationComponent::ClearGameplayTags()
{
	CurrentGameplayTags.Reset();
	PreviousGameplayTags.Reset();
}

void UAlsPhysicalAnimationComponent::RefreshBodyState(float DeltaTime)
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

namespace AlsPhysicalAnimationTagCombination
{
	struct FIterator
	{
		const struct FContainer* Container;
		TArray<FName> CurrentNames;
		TArray<int> Indices;

		const TArray<FName>& operator*() const { return CurrentNames; }
		FIterator& operator++();
		bool operator!=(const FIterator& lhs) const { return Container != Container || Indices != lhs.Indices || CurrentNames != lhs.CurrentNames; }
	};

	struct FContainer
	{
		TArray<FName> SourceNames;
		FIterator begin() const { return {this, SourceNames}; }
		FIterator end() const { return {nullptr}; }
	};

	FIterator& FIterator::operator++()
	{
		if (!Container)
		{
			CurrentNames.Reset();
			Indices.Reset();
			return *this;
		}

		auto SourceNum{Container->SourceNames.Num()};

		// ex. SourceNum = 5
		// SourceNum - IndicesNum(1) = 4
		// 11110 0
		// 11101 1
		// 11011 2
		// 10111 3
		// 01111 4
		// 
		// SourceNum - IndicesNum(2) = 3
		// 11100 0 0
		// 11010 0 1
		// 11001 1 1
		// 10110 0 2
		// 10101 1 2
		// 10011 2 2
		// 01110 0 3
		// 01101 1 3
		// 01011 2 3
		// 00111 3 3
		// 
		// SourceNum - IndicesNum(3) = 2
		// 11000 0 0 0
		// 10100 0 0 1
		// 10010 0 1 1
		// 10001 1 1 1
		// 01100 0 0 2
		// 01010 0 1 2
		// 01001 1 1 2
		// 00110 0 2 2
		// 00101 1 2 2
		// 00011 2 2 2
		// 
		// SourceNum - IndicesNum(4) = 1
		// 10000 0 0 0 0
		// 01000 0 0 0 1
		// 00100 0 0 1 1
		// 00010 0 1 1 1
		// 00001 1 1 1 1

		if (Indices.IsEmpty())
		{
			Indices.Push(0);
		}
		else
		{
			bool NextDepth = false;
			auto IndicesNum{Indices.Num()};
			for (int Index = 0; Index < IndicesNum; ++Index)
			{
				if (Index + 1 < IndicesNum)
				{
					if (Indices[Index] < Indices[Index + 1])
					{
						Indices[Index]++;
						break;
					}
					else
					{
						Indices[Index] = 0;
					}
				}
				else
				{
					if (Indices[Index] < SourceNum - IndicesNum)
					{
						Indices[Index]++;
					}
					else
					{
						Indices[Index] = 0;
						NextDepth = true;
					}
				}
			}
			if (NextDepth)
			{
				if(IndicesNum + 1 < SourceNum)
				{
					Indices.Push(0);
				}
				else
				{
					Container = nullptr;
					CurrentNames.Reset();
					Indices.Reset();
					return *this;
				}
			}
		}

		CurrentNames = Container->SourceNames;

		if (SourceNum > 1)
		{
			for (auto Index : Indices)
			{
				CurrentNames.RemoveAt(CurrentNames.Num() - 1 - Index);
			}
		}

		return *this;
	}
}

void UAlsPhysicalAnimationComponent::SelectProfile()
{
	using namespace AlsPhysicalAnimationTagCombination;
	TArray<FName> NextProfileNames;
	FName NextBaseProfileName;
	TArray<FName> NextAdditiveProfileNames;
	TArray<FName> NextMultiplyProfileNames;
	TStringBuilder<256> StringBuilder;
	TStringBuilder<256> AdditionalStringBuilder;

	FName RagdollingModeName{LatestRagdolling.IsValid() ? UAlsUtility::GetSimpleTagName(LatestRagdolling->AbilityTags.First()) : NAME_None};

	FContainer Container;

	for (auto& Mask : GameplayTagMasks)
	{
		auto Name{UAlsUtility::GetSimpleTagName(CurrentGameplayTags.Filter(Mask).First())};
		if (Name.IsValid() && !Name.IsNone())
		{
			Container.SourceNames.Add(Name);
		}
	}

	for (auto& Names : Container)
	{
		if(bRagdolling && !Names.Contains(RagdollingModeName))
		{
			continue;
		}

		for(auto& Name : Names)
		{
			if (StringBuilder.Len() > 0)
			{
				StringBuilder << TEXT(":");
			}
			StringBuilder << Name;
		}

		UE_LOG(LogAls, VeryVerbose, TEXT("Try Physical Animation Profile '%s'"), *StringBuilder);

		// determin base profile

		if (NextBaseProfileName.IsNone())
		{
			FName ProfileName(StringBuilder, FNAME_Find);
			if (IsProfileExist(ProfileName))
			{
				NextBaseProfileName = ProfileName;
			}
		}

		// add additive profile if exists

		AdditionalStringBuilder << TEXT("+");
		AdditionalStringBuilder.Append(StringBuilder);
		FName AdditiveProfileName(AdditionalStringBuilder, FNAME_Find);
		AdditionalStringBuilder.Reset();
		if (IsProfileExist(AdditiveProfileName))
		{
			NextAdditiveProfileNames.Add(AdditiveProfileName);
		}

		// add multiply profile if exists

		AdditionalStringBuilder << TEXT("*");
		AdditionalStringBuilder.Append(StringBuilder);
		FName MultiplyProfileName(AdditionalStringBuilder, FNAME_Find);
		AdditionalStringBuilder.Reset();
		if (IsProfileExist(MultiplyProfileName))
		{
			NextMultiplyProfileNames.Add(MultiplyProfileName);
		}

		StringBuilder.Reset();
	}

	if (NextBaseProfileName.IsNone() && bRagdolling && IsProfileExist(RagdollingModeName))
	{
		NextBaseProfileName = RagdollingModeName;
	}
	if (NextBaseProfileName.IsNone() && IsProfileExist(UAlsConstants::DefaultPAProfileName()))
	{
		NextBaseProfileName = UAlsConstants::DefaultPAProfileName();
	}

	if (NextBaseProfileName.IsNone())
	{
		NextAdditiveProfileNames.Reset();
		NextMultiplyProfileNames.Reset();
	}
	else
	{
		NextProfileNames.Add(NextBaseProfileName);
		NextProfileNames.Append(NextAdditiveProfileNames);
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
		CurrentProfileNames = NextProfileNames;

		for (const auto& NextMultiplyProfileName : NextMultiplyProfileNames)
		{
			ApplyPhysicalAnimationProfileBelow(NAME_None, NextMultiplyProfileName);
			GetSkeletalMesh()->SetConstraintProfileForAll(NextMultiplyProfileName);
		}
		CurrentMultiplyProfileNames = NextMultiplyProfileNames;
	}
}

void UAlsPhysicalAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	Character->GetAbilitySystemComponent()->AbilityActivatedCallbacks.AddUObject(this, &ThisClass::OnAbilityActivated);
}

void UAlsPhysicalAnimationComponent::OnAbilityActivated(class UGameplayAbility* GameplayAbility)
{
	auto Ragdolling{Cast<UAlsGameplayAbility_Ragdolling>(GameplayAbility)};
	if (Ragdolling)
	{
		LatestRagdolling = Ragdolling;
	}
}

void UAlsPhysicalAnimationComponent::Refresh(const AAlsCharacter* Character)
{
	if (LatestRagdolling.IsValid() && LatestRagdolling->bCancelRequested)
	{
		LatestRagdolling->Cancel();
	}

	// Apply special behaviour when changed Ragdolling state

	if (LatestRagdolling.IsValid() && LatestRagdolling->IsActive())
	{
		if (!bRagdolling)
		{
			bRagdolling = true;
			GetSkeletalMesh()->SetAllBodiesBelowSimulatePhysics(UAlsConstants::PelvisBoneName(), true);
			GetSkeletalMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
			ApplyPhysicalAnimationProfileBelow(NAME_None, NAME_None, true, true);
		}
		bRagdollingFreezed = LatestRagdolling->bFreezing;
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
			bRagdollingFreezed = false;
		}
		LatestRagdolling.Reset();
	}

	FGameplayTagContainer TempContainer;
	Character->GetOwnedGameplayTags(TempContainer);
	FGameplayTagContainer TempMaskContainer;
	for (auto& Container : GameplayTagMasks)
	{
		TempMaskContainer.AppendTags(Container);
	}
	CurrentGameplayTags.Reset();
	CurrentGameplayTags.AppendMatchingTags(TempContainer, TempMaskContainer);
}

void UAlsPhysicalAnimationComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
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
	else if(NeedsProfileChange())
	{
		SelectProfile();
	}

	// Update PhysicsBlendWeight and Collision settings

	if (!bRagdolling || !bRagdollingFreezed)
	{
		RefreshBodyState(DeltaTime);
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
