#include "AlsCharacter.h"

#include "DisplayDebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "AlsPhysicalAnimationComponent.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"

#define LOCTEXT_NAMESPACE "AlsCharacterDebug"

#if !UE_BUILD_SHIPPING
void AAlsCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation)
{
	const auto Scale{FMath::Min(Canvas->SizeX / (1280.0f * Canvas->GetDPIScale()), Canvas->SizeY / (720.0f * Canvas->GetDPIScale()))};

	const auto RowOffset{12.0f * Scale};
	const auto ColumnOffset{200.0f * Scale};

	auto MaxVerticalLocation{VerticalLocation};
	auto HorizontalLocation{5.0f * Scale};

	static const auto DebugModeHeaderText{LOCTEXT("DebugModeHeader", "Debug mode is enabled! Press (Shift + 0) to disable.")};

	DisplayDebugHeader(Canvas, DebugModeHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);

	VerticalLocation += RowOffset;
	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	if (!DisplayInfo.IsDisplayOn(UAlsConstants::CurvesDebugDisplayName()) &&
	    !DisplayInfo.IsDisplayOn(UAlsConstants::StateDebugDisplayName()) &&
	    !DisplayInfo.IsDisplayOn(UAlsConstants::ShapesDebugDisplayName()) &&
	    !DisplayInfo.IsDisplayOn(UAlsConstants::TracesDebugDisplayName()) &&
	    !DisplayInfo.IsDisplayOn(UAlsConstants::MantlingDebugDisplayName()) &&
	    !DisplayInfo.IsDisplayOn(UAlsConstants::PADebugDisplayName()))
	{
		VerticalLocation = MaxVerticalLocation;

		OnDisplayDebug.Broadcast(Canvas, DisplayInfo, Unused, VerticalLocation);

		Super::DisplayDebug(Canvas, DisplayInfo, Unused, VerticalLocation);
		return;
	}

	const auto InitialVerticalLocation{VerticalLocation};

	static const auto CurvesHeaderText{FText::AsCultureInvariant(FString{TEXTVIEW("Als.Curves (Shift + 1)")})};

	if (DisplayInfo.IsDisplayOn(UAlsConstants::CurvesDebugDisplayName()))
	{
		DisplayDebugHeader(Canvas, CurvesHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
		DisplayDebugCurves(Canvas, Scale, HorizontalLocation, VerticalLocation);

		MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation + RowOffset);
		VerticalLocation = InitialVerticalLocation;
		HorizontalLocation += ColumnOffset;
	}
	else
	{
		DisplayDebugHeader(Canvas, CurvesHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);

		VerticalLocation += RowOffset;
	}

	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	static const auto StateHeaderText{FText::AsCultureInvariant(FString{TEXTVIEW("Als.State (Shift + 2)")})};

	if (DisplayInfo.IsDisplayOn(UAlsConstants::StateDebugDisplayName()))
	{
		DisplayDebugHeader(Canvas, StateHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
		DisplayDebugState(Canvas, Scale, HorizontalLocation, VerticalLocation);
	}
	else
	{
		DisplayDebugHeader(Canvas, StateHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);
	}

	VerticalLocation += RowOffset;
	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	static const auto ShapesHeaderText{FText::AsCultureInvariant(FString{TEXTVIEW("Als.Shapes (Shift + 3)")})};

	if (DisplayInfo.IsDisplayOn(UAlsConstants::ShapesDebugDisplayName()))
	{
		DisplayDebugHeader(Canvas, ShapesHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
		DisplayDebugShapes(Canvas, Scale, HorizontalLocation, VerticalLocation);
	}
	else
	{
		DisplayDebugHeader(Canvas, ShapesHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);
	}

	VerticalLocation += RowOffset;
	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	static const auto TracesHeaderText{FText::AsCultureInvariant(FString{TEXTVIEW("Als.Traces (Shift + 4)")})};

	if (DisplayInfo.IsDisplayOn(UAlsConstants::TracesDebugDisplayName()))
	{
		DisplayDebugHeader(Canvas, TracesHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
		DisplayDebugTraces(Canvas, Scale, HorizontalLocation, VerticalLocation);
	}
	else
	{
		DisplayDebugHeader(Canvas, TracesHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);
	}

	VerticalLocation += RowOffset;
	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	static const auto MantlingHeaderText{FText::AsCultureInvariant(FString{TEXTVIEW("Als.Mantling (Shift + 5)")})};

	if (DisplayInfo.IsDisplayOn(UAlsConstants::MantlingDebugDisplayName()))
	{
		DisplayDebugHeader(Canvas, MantlingHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
		DisplayDebugMantling(Canvas, Scale, HorizontalLocation, VerticalLocation);
	}
	else
	{
		DisplayDebugHeader(Canvas, MantlingHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);
	}

	static const auto PAHeaderText{FText::AsCultureInvariant(FString{TEXTVIEW("Als.PhysicalAnimation (Shift + 6)")})};

	if (DisplayInfo.IsDisplayOn(UAlsConstants::PADebugDisplayName()))
	{
		DisplayDebugHeader(Canvas, PAHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);
		PhysicalAnimation->DisplayDebug(Canvas, DisplayInfo, HorizontalLocation, VerticalLocation);
	}
	else
	{
		DisplayDebugHeader(Canvas, PAHeaderText, {0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, VerticalLocation);
	}

	VerticalLocation += RowOffset;
	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	VerticalLocation = MaxVerticalLocation;

	OnDisplayDebug.Broadcast(Canvas, DisplayInfo, Unused, VerticalLocation);

	Super::DisplayDebug(Canvas, DisplayInfo, Unused, VerticalLocation);
}

void AAlsCharacter::DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor,
                                       const float Scale, const float HorizontalLocation, float& VerticalLocation)
{
	FCanvasTextItem Text{
		{HorizontalLocation, VerticalLocation},
		HeaderText,
		GEngine->GetMediumFont(),
		HeaderColor
	};

	Text.Scale = {Scale, Scale};
	Text.EnableShadow(FLinearColor::Black);

	Text.Draw(Canvas->Canvas);

	VerticalLocation += 15.0f * Scale;
}

TArray<FName> AAlsCharacter::CurveNames;

void AAlsCharacter::InitializeCurveNames()
{
	CurveNames.Reset();
	// gave up
	//UClass* _Class = UAlsConstants::StaticClass();
	//for (TFieldIterator<UFunction> FuncIt(_Class); FuncIt; ++FuncIt)
	//{
	//	if (FuncIt->HasAnyFunctionFlags(FUNC_Static))
	//	{
	//		const auto ReturnDisplayName = FuncIt->FindMetaData("ReturnDisplayName");
	//		if (ReturnDisplayName && ReturnDisplayName->Equals("Curve Name"))
	//		{
	//			const FName& CurveName = FuncIt->CallStaticFunction();
	//			CurveNames.AddUniqueUnique(CurveName);
	//		}
	//	}
	//}
	
	// Layering Animation Curves
	CurveNames.AddUnique(UAlsConstants::LayerHeadCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerHeadAdditiveCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerHeadSlotCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmLeftCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmLeftAdditiveCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmLeftLocalSpaceCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmLeftSlotCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmRightCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmRightAdditiveCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmRightLocalSpaceCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerArmRightSlotCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerHandLeftCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerHandRightCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerSpineCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerSpineAdditiveCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerSpineSlotCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerPelvisCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerPelvisSlotCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerLegsCurveName());
	CurveNames.AddUnique(UAlsConstants::LayerLegsSlotCurveName());
	CurveNames.AddUnique(UAlsConstants::HandLeftIkCurveName());
	CurveNames.AddUnique(UAlsConstants::HandRightIkCurveName());
	CurveNames.AddUnique(UAlsConstants::ViewBlockCurveName());
	CurveNames.AddUnique(UAlsConstants::AllowAimingCurveName());
	CurveNames.AddUnique(UAlsConstants::HipsDirectionLockCurveName());

	// Pose Animation Curves
	CurveNames.AddUnique(UAlsConstants::PoseGaitCurveName());
	CurveNames.AddUnique(UAlsConstants::PoseMovingCurveName());
	CurveNames.AddUnique(UAlsConstants::PoseStandingCurveName());
	CurveNames.AddUnique(UAlsConstants::PoseCrouchingCurveName());
	CurveNames.AddUnique(UAlsConstants::PoseGroundedCurveName());
	CurveNames.AddUnique(UAlsConstants::PoseInAirCurveName());

	// Feet Animation Curves
	CurveNames.AddUnique(UAlsConstants::FootLeftIkCurveName());
	CurveNames.AddUnique(UAlsConstants::FootLeftLockCurveName());
	CurveNames.AddUnique(UAlsConstants::FootRightIkCurveName());
	CurveNames.AddUnique(UAlsConstants::FootRightLockCurveName());
	CurveNames.AddUnique(UAlsConstants::FootPlantedCurveName());
	CurveNames.AddUnique(UAlsConstants::FeetCrossingCurveName());

	// Other Animation Curves
	CurveNames.AddUnique(UAlsConstants::RotationYawSpeedCurveName());
	CurveNames.AddUnique(UAlsConstants::RotationYawOffsetCurveName());
	CurveNames.AddUnique(UAlsConstants::AllowTransitionsCurveName());
	CurveNames.AddUnique(UAlsConstants::SprintBlockCurveName());
	CurveNames.AddUnique(UAlsConstants::GroundPredictionBlockCurveName());
	CurveNames.AddUnique(UAlsConstants::FootstepSoundBlockCurveName());

	// Physical Animation Curves
	CurveNames.AddUnique(UAlsConstants::PALockArmLeftCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockArmRightCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockLegLeftCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockLegRightCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockHandLeftCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockHandRightCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockFootLeftCurveName());
	CurveNames.AddUnique(UAlsConstants::PALockFootRightCurveName());
}

void AAlsCharacter::DisplayDebugCurves(const UCanvas* Canvas, const float Scale,
                                       const float HorizontalLocation, float& VerticalLocation) const
{
	VerticalLocation += 4.0f * Scale;

	FCanvasTextItem Text{
		FVector2D::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetSmallFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale * 0.75f, Scale * 0.75f};
	Text.EnableShadow(FLinearColor::Black);

	const auto RowOffset{10.0f * Scale};
	const auto ColumnOffset{145.0f * Scale};

	if (CurveNames.Num() == 0)
	{
		InitializeCurveNames();
	}

	TStringBuilder<32> CurveValueBuilder;

	TArray<FName> MeshCurveNames;
	GetMesh()->GetSkeletalMeshAsset()->GetSkeleton()->GetCurveMetaDataNames(MeshCurveNames);
	for (const auto& CurveName : CurveNames)
	{
		// Skip if CurveName doesn't exist in UAlsConstants's Animation Curve Name static members.
		if (!MeshCurveNames.Contains(CurveName))
		{
			continue;
		}

		const auto CurveValue{GetMesh()->GetAnimInstance()->GetCurveValue(CurveName)};

		Text.SetColor(FMath::Lerp(FLinearColor::Gray, FLinearColor::White, UAlsMath::Clamp01(CurveValue)));

		Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(CurveName.ToString(), false));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

		CurveValueBuilder.Appendf(TEXT("%.2f"), CurveValue);

		Text.Text = FText::AsCultureInvariant(FString{CurveValueBuilder});
		Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

		CurveValueBuilder.Reset();

		VerticalLocation += RowOffset;
	}
}

void AAlsCharacter::DisplayDebugState(const UCanvas* Canvas, const float Scale,
                                      const float HorizontalLocation, float& VerticalLocation) const
{
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
	const auto ColumnOffset{120.0f * Scale};

	static const auto DesiredViewModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, DesiredViewMode), false))
	};

	Text.Text = DesiredViewModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(DesiredViewMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto ViewModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, ViewMode), false))
	};

	Text.Text = ViewModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(ViewMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto LocomotionModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, LocomotionMode), false))
	};

	Text.Text = LocomotionModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(LocomotionMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto DesiredAimingText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, AimingMode), true))
	};

	Text.Text = DesiredAimingText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(AimingMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto DesiredRotationModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, DesiredRotationMode), false))
	};

	Text.Text = DesiredRotationModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(
		FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(DesiredRotationMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto RotationModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, RotationMode), false))
	};

	Text.Text = RotationModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(RotationMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto DesiredStanceText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, DesiredStance), false))
	};

	Text.Text = DesiredStanceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(DesiredStance).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto StanceText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, Stance), false))
	};

	Text.Text = StanceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(Stance).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto DesiredGaitText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, DesiredGait), false))
	};

	Text.Text = DesiredGaitText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(DesiredGait).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto GaitText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, Gait), false))
	};

	Text.Text = GaitText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(Gait).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto OverlayModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(TEXT("OverlayMode"), false))
	};

	Text.Text = OverlayModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(GetOverlayMode()).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto LocomotionActionText{
		FText::AsCultureInvariant(FName::NameToDisplayString(TEXT("LocomotionAction"), false))
	};

	Text.Text = LocomotionActionText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(UAlsUtility::GetSimpleTagName(GetLocomotionAction()).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;
}

void AAlsCharacter::DisplayDebugShapes(const UCanvas* Canvas, const float Scale,
                                       const float HorizontalLocation, float& VerticalLocation) const
{
	VerticalLocation += 4.0f * Scale;

	TStringBuilder<256> DebugStringBuilder;

	FCanvasTextItem Text{
		FVector2D::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetMediumFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale * 0.75f, Scale * 0.75f};
	Text.EnableShadow(FLinearColor::Black);

	const auto RowOffset{12.0f * Scale};
	const auto ColumnOffset{120.0f * Scale};

	static const auto ViewRotationText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(FAlsViewState, Rotation), false))
	};

	auto Color{FLinearColor::Red};
	Text.SetColor(Color);

	Text.Text = ViewRotationText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	DebugStringBuilder << TEXTVIEW("R:");
	DebugStringBuilder.Appendf(TEXT("%.2f"), ViewState.Rotation.Roll);
	DebugStringBuilder << TEXTVIEW(" P:");
	DebugStringBuilder.Appendf(TEXT("%.2f"), ViewState.Rotation.Pitch);
	DebugStringBuilder << TEXTVIEW(" Y:");
	DebugStringBuilder.Appendf(TEXT("%.2f"), ViewState.Rotation.Yaw);

	Text.Text = FText::AsCultureInvariant(FString{DebugStringBuilder});
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	DebugStringBuilder.Reset();

#if ENABLE_DRAW_DEBUG
	DrawDebugCone(GetWorld(), GetPawnViewLocation(),
	              ViewState.Rotation.Vector(), 100.0f, FMath::DegreesToRadians(15.0f), FMath::DegreesToRadians(15.0f),
	              8, Color.ToFColor(true), false, -1.0f, SDPG_World, 1.0f);
	DrawDebugCone(GetWorld(), GetPawnViewLocation(),
	              ViewState.LookRotation.Vector(), 100.0f, FMath::DegreesToRadians(10.0f), FMath::DegreesToRadians(10.0f),
	              8, FLinearColor::Blue.ToFColor(true), false, -1.0f, SDPG_World, 1.0f);
#endif

	VerticalLocation += RowOffset;

	static const auto InputYawAngleText{
		FText::AsCultureInvariant(
			FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(FAlsLocomotionState, InputYawAngle), false))
	};

	Color = LocomotionState.bHasInput ? FLinearColor{1.0f, 0.5f, 0.0f} : FLinearColor{0.5f, 0.25f, 0.0f};
	Text.SetColor(Color);

	Text.Text = InputYawAngleText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	DebugStringBuilder.Appendf(TEXT("%.2f"), LocomotionState.InputYawAngle);

	Text.Text = FText::AsCultureInvariant(FString{DebugStringBuilder});
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	DebugStringBuilder.Reset();

#if ENABLE_DRAW_DEBUG
	const auto FeetLocation{LocomotionState.Location - FVector{0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()}};

	DrawDebugDirectionalArrow(GetWorld(),
	                          FeetLocation + FVector{0.0f, 0.0f, 3.0f},
	                          FeetLocation + FVector{0.0f, 0.0f, 3.0f} +
	                          UAlsMath::AngleToDirectionXY(LocomotionState.InputYawAngle) * 50.0f,
	                          50.0f, Color.ToFColor(true), false, -1.0f, SDPG_World, 3.0f);
#endif

	VerticalLocation += RowOffset;

	static const auto SpeedText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(FAlsLocomotionState, Speed), false))
	};

	Color = LocomotionState.bHasSpeed ? FLinearColor{0.75f, 0.0f, 1.0f} : FLinearColor{0.375f, 0.0f, 0.5f};
	Text.SetColor(Color);

	Text.Text = SpeedText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	DebugStringBuilder.Appendf(TEXT("%.2f"), LocomotionState.Speed);

	Text.Text = FText::AsCultureInvariant(FString{DebugStringBuilder});
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	DebugStringBuilder.Reset();

	VerticalLocation += RowOffset;

	static const auto VelocityDirectionText{FText::AsCultureInvariant(FString{TEXTVIEW("Velocity Direction")})};

	const auto VelocityDirection{LocomotionState.Velocity.GetSafeNormal()};

	Text.Text = VelocityDirectionText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	DebugStringBuilder << TEXTVIEW("X:");
	DebugStringBuilder.Appendf(TEXT("%.2f"), VelocityDirection.X);
	DebugStringBuilder << TEXTVIEW(" Y:");
	DebugStringBuilder.Appendf(TEXT("%.2f"), VelocityDirection.Y);
	DebugStringBuilder << TEXTVIEW(" Z:");
	DebugStringBuilder.Appendf(TEXT("%.2f"), VelocityDirection.Z);

	Text.Text = FText::AsCultureInvariant(FString{DebugStringBuilder});
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	DebugStringBuilder.Reset();

	VerticalLocation += RowOffset;

	static const auto VelocityYawAngleText{
		FText::AsCultureInvariant(
			FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(FAlsLocomotionState, VelocityYawAngle), false))
	};

	Text.Text = VelocityYawAngleText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	DebugStringBuilder.Appendf(TEXT("%.2f"), LocomotionState.VelocityYawAngle);

	Text.Text = FText::AsCultureInvariant(FString{DebugStringBuilder});
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	DebugStringBuilder.Reset();

#if ENABLE_DRAW_DEBUG
	DrawDebugDirectionalArrow(GetWorld(),
	                          FeetLocation,
	                          FeetLocation +
	                          UAlsMath::AngleToDirectionXY(LocomotionState.VelocityYawAngle) *
	                          FMath::GetMappedRangeValueClamped(FVector2f{0.0f, GetCharacterMovement()->GetMaxSpeed()},
	                                                            {50.0f, 75.0f}, LocomotionState.Speed),
	                          50.0f, Color.ToFColor(true), false, -1.0f, SDPG_World, 3.0f);
#endif

	VerticalLocation += RowOffset;

	static const auto TargetYawAngleText{
		FText::AsCultureInvariant(
			FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(FAlsLocomotionState, SmoothTargetYawAngle), false))
	};

	Color = {0.0f, 0.75f, 1.0f};
	Text.SetColor(Color);

	Text.Text = TargetYawAngleText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	DebugStringBuilder.Appendf(TEXT("%.2f"), LocomotionState.SmoothTargetYawAngle);

	Text.Text = FText::AsCultureInvariant(FString{DebugStringBuilder});
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	DebugStringBuilder.Reset();

#if ENABLE_DRAW_DEBUG
	DrawDebugDirectionalArrow(GetWorld(),
	                          FeetLocation + FVector{0.0f, 0.0f, 6.0f},
	                          FeetLocation + FVector{0.0f, 0.0f, 6.0f} +
	                          UAlsMath::AngleToDirectionXY(LocomotionState.SmoothTargetYawAngle) * 50.0f,
	                          50.0f, Color.ToFColor(true), false, -1.0f, SDPG_World, 3.0f);
#endif

	VerticalLocation += RowOffset;

#if ENABLE_DRAW_DEBUG
	DrawDebugCapsule(GetWorld(), LocomotionState.Location, GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
	                 GetCapsuleComponent()->GetScaledCapsuleRadius(), LocomotionState.RotationQuaternion,
	                 FColor::Green, false, -1.0f, SDPG_World, 1.0f);
#endif
}

void AAlsCharacter::DisplayDebugTraces(const UCanvas* Canvas, const float Scale,
                                       const float HorizontalLocation, float& VerticalLocation) const
{
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

	static const auto FootOffsetTraceText{LOCTEXT("FootOffsetTrace", "Foot Offset")};

	Text.SetColor({0.0f, 0.75f, 1.0f});

	Text.Text = FootOffsetTraceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto GroundPredictionTraceText{LOCTEXT("GroundPredictionTrace", "Ground Prediction")};

	Text.SetColor({0.75f, 0.0f, 1.0f});

	Text.Text = GroundPredictionTraceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto FootstepEffectsTraceText{LOCTEXT("FootstepEffectsTrace", "Footstep Effects")};

	Text.SetColor(FLinearColor::Red);

	Text.Text = FootstepEffectsTraceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;
}

void AAlsCharacter::DisplayDebugMantling(const UCanvas* Canvas, const float Scale,
                                         const float HorizontalLocation, float& VerticalLocation) const
{
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

	static const auto ForwardTraceText{LOCTEXT("ForwardTrace", "Forward Trace")};

	Text.SetColor({0.0f, 0.75f, 1.0f});

	Text.Text = ForwardTraceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto DownwardTraceText{LOCTEXT("DownwardTrace", "Downward Trace")};

	Text.SetColor({0.75f, 0.0f, 1.0f});

	Text.Text = DownwardTraceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto FailedTargetLocationOverlapText{LOCTEXT("FailedTargetocationOverlap", "Failed Target Location Overlap")};

	Text.SetColor(FLinearColor::Red);

	Text.Text = FailedTargetLocationOverlapText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto FailedStartLocationOverlapText{LOCTEXT("FailedStartLocationOverlap", "Failed Start Location Overlap")};

	Text.SetColor({1.0f, 0.5f, 0.0f});

	Text.Text = FailedStartLocationOverlapText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	VerticalLocation += RowOffset;
}
#endif // !UE_BUILD_SHIPPING

#undef LOCTEXT_NAMESPACE
