#include "AlsCameraSkeletalMeshComponent.h"

#include "AlsCameraSettings.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Curves/CurveFloat.h"
#include "Utility/AlsCameraConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCameraSkeletalMeshComponent)

UAlsCameraSkeletalMeshComponent::UAlsCameraSkeletalMeshComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bAutoActivate = false;
	bTickInEditor = false;
	bHiddenInGame = true;

	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	SetAllowClothActors(false);
	SetCastShadow(false);
}

void UAlsCameraSkeletalMeshComponent::OnRegister()
{
	Character = Cast<AAlsCharacter>(GetOwner());

	Super::OnRegister();
}

void UAlsCameraSkeletalMeshComponent::Activate(const bool bReset)
{
	Super::Activate(bReset);

	SetComponentTickEnabled(true);

	Camera->Activate();
	Camera->SetComponentTickEnabled(true);

	if (!bReset && !ShouldActivate())
	{
		return;
	}

	TickCamera(0.0f, false);
}

void UAlsCameraSkeletalMeshComponent::Deactivate()
{
	SetComponentTickEnabled(false);
	Camera->SetComponentTickEnabled(false);
	Camera->Deactivate();
	Super::Deactivate();
}

void UAlsCameraSkeletalMeshComponent::RegisterComponentTickFunctions(const bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	// Tick after the owner to have access to the most up-to-date character state.

	AddTickPrerequisiteActor(GetOwner());
}

void UAlsCameraSkeletalMeshComponent::BeginPlay()
{
	ALS_ENSURE(IsValid(GetAnimInstance()));
	ALS_ENSURE(Character.IsValid());
	ALS_ENSURE(Camera.IsValid());
	ALS_ENSURE(IsValid(Settings));

	bFPP = FAnimWeight::IsFullWeight(UAlsMath::Clamp01(GetAnimInstance()->GetCurveValue(UAlsCameraConstants::FirstPersonOverrideCurveName())));
	bPreviousRightShoulder = bRightShoulder;
	PreviousViewMode = Character->GetViewMode();

	Super::BeginPlay();

	Character->OnChangedPerspective(bFPP);
}

void UAlsCameraSkeletalMeshComponent::TickComponent(float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsValid(Settings) && Settings->bIgnoreTimeDilation)
	{
		// Use the previous global time dilation, as this frame's delta time may not yet be affected
		// by the current global time dilation, and thus unscaling will produce the wrong delta time.

		const auto TimeDilation{PreviousGlobalTimeDilation * GetOwner()->CustomTimeDilation};

		DeltaTime = TimeDilation > UE_SMALL_NUMBER ? DeltaTime / TimeDilation : GetWorld()->DeltaRealTimeSeconds;
	}

	PreviousGlobalTimeDilation = GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Skip camera tick until parallel animation evaluation completes.

	if (Camera.IsValid() && !IsRunningParallelEvaluation())
	{
		TickCamera(DeltaTime);
	}
}

void UAlsCameraSkeletalMeshComponent::CompleteParallelAnimationEvaluation(const bool bDoPostAnimationEvaluation)
{
	Super::CompleteParallelAnimationEvaluation(bDoPostAnimationEvaluation);

	if (Camera.IsValid())
	{
		TickCamera(GetAnimInstance()->GetDeltaSeconds());
	}
}

void UAlsCameraSkeletalMeshComponent::SetCameraComponent(UCameraComponent* NewCameraComponent)
{
	if (Camera.IsValid())
	{
		Camera->RemoveTickPrerequisiteComponent(this);
	}
	Camera = NewCameraComponent;
	Camera->AddTickPrerequisiteComponent(this);
}

FVector UAlsCameraSkeletalMeshComponent::GetFirstPersonCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(Settings->FirstPerson.CameraSocketName);
}

FVector UAlsCameraSkeletalMeshComponent::GetEyeCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(Settings->FirstPerson.bLeftDominantEye
												   ? Settings->FirstPerson.LeftEyeCameraSocketName
												   : Settings->FirstPerson.RightEyeCameraSocketName);
}

void UAlsCameraSkeletalMeshComponent::CalculateAimingFirstPersonCamera(float AimingAmount, const FRotator& TargetRotation)
{
	if (AimingAmount > 0.0f && Character->HasSight())
	{
		const auto EyeCameraLocation{GetEyeCameraLocation()};
		FVector SightLoc;
		FRotator SightRot;
		Character->GetSightLocAndRot(SightLoc, SightRot);
		SightLoc = FVector::PointPlaneProject(SightLoc, EyeCameraLocation, SightRot.Vector());
		if (AimingAmount >= 1.0f)
		{
			CameraLocation = SightLoc - SightRot.Vector() * Settings->FirstPerson.RetreatDistance;
			CameraRotation = SightRot;
			return;
		}
		else
		{
			auto EyeAlpha = UAlsMath::Clamp01(AimingAmount / Settings->FirstPerson.ADSThreshold);
			auto SightAlpha = UAlsMath::Clamp01((AimingAmount - Settings->FirstPerson.ADSThreshold) /
				(1.0f - Settings->FirstPerson.ADSThreshold));
			CameraRotation = FRotator(FQuat::Slerp(TargetRotation.Quaternion(), SightRot.Quaternion(), SightAlpha));
			auto Offset = CameraRotation.Vector() * Settings->FirstPerson.RetreatDistance;
			auto EyeLoc = FMath::Lerp(GetFirstPersonCameraLocation(), EyeCameraLocation, EyeAlpha);
			CameraLocation = FMath::Lerp(EyeLoc, SightLoc, SightAlpha) - Offset;
			return;
		}
	}

	auto Offset = TargetRotation.Vector() * Settings->FirstPerson.RetreatDistance;
	CameraLocation = GetFirstPersonCameraLocation() - Offset;
	CameraRotation = TargetRotation;
}

FVector UAlsCameraSkeletalMeshComponent::GetThirdPersonPivotLocation() const
{
	const auto* Mesh{Character->GetMesh()};

	FVector FirstPivotLocation;

	if (!IsValid(Mesh->GetAttachParent()) && Settings->ThirdPerson.FirstPivotSocketName == UAlsConstants::RootBoneName())
	{
		// The root bone location usually remains fixed when the mesh is detached, so use the capsule's bottom location here as a fallback.

		FirstPivotLocation = Character->GetRootComponent()->GetComponentLocation();
		FirstPivotLocation.Z -= Character->GetRootComponent()->Bounds.BoxExtent.Z;
	}
	else
	{
		FirstPivotLocation = Mesh->GetSocketLocation(Settings->ThirdPerson.FirstPivotSocketName);
	}

	if (Settings->ThirdPerson.bApplyVelocityLead)
	{
		FirstPivotLocation += CurrentLeadVector;
	}

	return (FirstPivotLocation + Mesh->GetSocketLocation(Settings->ThirdPerson.SecondPivotSocketName)) * 0.5f;
}

FVector UAlsCameraSkeletalMeshComponent::GetThirdPersonTraceStartLocation() const
{
	return Character->GetMesh()->GetSocketLocation(bRightShoulder
		                                           ? Settings->ThirdPerson.TraceShoulderRightSocketName
		                                           : Settings->ThirdPerson.TraceShoulderLeftSocketName);
}

void UAlsCameraSkeletalMeshComponent::TickCamera(const float DeltaTime, bool bAllowLag)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UAlsCameraSkeletalMeshComponent::TickCamera()"), STAT_UAlsCameraSkeletalMeshComponent_TickCamera, STATGROUP_Als)

	if (!IsValid(GetAnimInstance()) || !IsValid(Settings) || !Character.IsValid())
	{
		return;
	}

	ALS_ENSURE_MESSAGE(!IsRunningParallelEvaluation(),
	                   TEXT("%hs should not be called during parallel animation evaluation, because accessing animation curves")
	                   TEXT(" causes the game thread to wait for the parallel task to complete, resulting in performance degradation."),
	                   __FUNCTION__);

#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebugCameraShapes{
		UAlsUtility::ShouldDisplayDebugForActor(GetOwner(), UAlsCameraConstants::CameraShapesDebugDisplayName())
	};
	const auto bDisplayDebugCameraTraces{
		UAlsUtility::ShouldDisplayDebugForActor(GetOwner(), UAlsCameraConstants::CameraTracesDebugDisplayName())
	};
#endif

	// Refresh movement base.

	const auto& BasedMovement{Character->GetBasedMovement()};
	const auto bMovementBaseHasRelativeRotation{BasedMovement.HasRelativeRotation()};

	auto MovementBaseLocation{FVector::ZeroVector};
	auto MovementBaseRotation{FQuat::Identity};

	if (bMovementBaseHasRelativeRotation)
	{
		MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
		                                              MovementBaseLocation, MovementBaseRotation);
	}

	if (BasedMovement.MovementBase != MovementBasePrimitive || BasedMovement.BoneName != MovementBaseBoneName)
	{
		MovementBasePrimitive = BasedMovement.MovementBase;
		MovementBaseBoneName = BasedMovement.BoneName;

		if (bMovementBaseHasRelativeRotation)
		{
			const auto MovementBaseRotationInverse{MovementBaseRotation.Inverse()};

			PivotMovementBaseRelativeLagLocation = MovementBaseRotationInverse.RotateVector(PivotLagLocation - MovementBaseLocation);
			CameraMovementBaseRelativeRotation = MovementBaseRotationInverse * CameraRotation.Quaternion();
		}
		else
		{
			PivotMovementBaseRelativeLagLocation = FVector::ZeroVector;
			CameraMovementBaseRelativeRotation = FQuat::Identity;
		}
	}
	
	const auto FirstPersonOverride{
		UAlsMath::Clamp01(GetAnimInstance()->GetCurveValue(UAlsCameraConstants::FirstPersonOverrideCurveName()))
	};

	const auto AimingAmount{Character->GetAimAmount()};

	UpdateADSCameraShake(FirstPersonOverride, AimingAmount);

	// Refresh CurrentLeadVector if needed.

	if (Settings->ThirdPerson.bApplyVelocityLead)
	{
		const FRotator CameraYawRotation{0.0f, CameraRotation.Yaw, 0.0f};
		auto LocalVelocity{CameraYawRotation.UnrotateVector(Character->GetAlsCharacterMovement()->Velocity) * Settings->ThirdPerson.VelocityLeadRate};
		auto LeadVector = Character->GetRotationMode() != AlsRotationModeTags::Aiming && !FAnimWeight::IsFullWeight(FirstPersonOverride) ?
			CameraYawRotation.RotateVector(LocalVelocity) : FVector::ZeroVector;
		CurrentLeadVector = FMath::VInterpTo(CurrentLeadVector, LeadVector, DeltaTime, Settings->ThirdPerson.VelocityLeadInterpSpeed);
		LatestCameraYawRotation = CameraYawRotation;
	}

	const auto CameraTargetRotation{Character->GetViewRotation()};

	const auto PreviousPivotTargetLocation{PivotTargetLocation};

	PivotTargetLocation = GetThirdPersonPivotLocation();

	if (FAnimWeight::IsFullWeight(FirstPersonOverride))
	{
		// Skip other calculations if the character is fully in first-person mode.

		PivotLagLocation = PivotTargetLocation;
		PivotLocation = PivotTargetLocation;
		bInAutoFPP = false;

		CalculateAimingFirstPersonCamera(AimingAmount, CameraTargetRotation);
		Camera->FieldOfView = Settings->FirstPerson.FOV;
		Camera->bPanoramic = Settings->FirstPerson.bPanoramic;
		Camera->PanoramicFieldOfView = Settings->FirstPerson.PanoramaFOV;
		Camera->PanoramaSideViewRate = Settings->FirstPerson.PanoramaSideViewRate;

		FocalLength = CalculateFocalLength();
		Character->SetLookRotation(Character->GetViewRotation());
		Camera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);

		if (!bFPP)
		{
			bFPP = true;
			// call Begin FPP Event
			Character->OnChangedPerspective(true);
		}
		return;
	}

	// Force disable camera lag if the character was teleported.

	bAllowLag &= Settings->TeleportDistanceThreshold <= 0.0f ||
				 FVector::DistSquared(PreviousPivotTargetLocation, PivotTargetLocation) <= FMath::Square(Settings->TeleportDistanceThreshold);

	// Calculate camera rotation.

	if (bMovementBaseHasRelativeRotation)
	{
		CameraRotation = (MovementBaseRotation * CameraMovementBaseRelativeRotation).Rotator();

		CameraRotation = CalculateCameraRotation(CameraTargetRotation, DeltaTime, bAllowLag);

		CameraMovementBaseRelativeRotation = MovementBaseRotation.Inverse() * CameraRotation.Quaternion();
	}
	else
	{
		CameraRotation = CalculateCameraRotation(CameraTargetRotation, DeltaTime, bAllowLag);
	}

	const FRotator CameraYawRotation{0.0f, CameraRotation.Yaw, 0.0f};

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraShapes)
	{
		UAlsUtility::DrawDebugSphereAlternative(GetWorld(), PivotTargetLocation, CameraYawRotation, 16.0f, FLinearColor::Green);
	}
#endif

	// Calculate pivot lag location. Get the pivot target location and interpolate using axis-independent lag for maximum control.

	if (bMovementBaseHasRelativeRotation)
	{
		PivotLagLocation = MovementBaseLocation + MovementBaseRotation.RotateVector(PivotMovementBaseRelativeLagLocation);

		PivotLagLocation = CalculatePivotLagLocation(CameraYawRotation.Quaternion(), DeltaTime, bAllowLag);

		PivotMovementBaseRelativeLagLocation = MovementBaseRotation.UnrotateVector(PivotLagLocation - MovementBaseLocation);
	}
	else
	{
		PivotLagLocation = CalculatePivotLagLocation(CameraYawRotation.Quaternion(), DeltaTime, bAllowLag);
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraShapes)
	{
		DrawDebugLine(GetWorld(), PivotLagLocation, PivotTargetLocation,
		              FLinearColor{1.0f, 0.5f, 0.0f}.ToFColor(true),
		              false, 0.0f, 0, UAlsUtility::DrawLineThickness);

		UAlsUtility::DrawDebugSphereAlternative(GetWorld(), PivotLagLocation, CameraYawRotation, 16.0f, {1.0f, 0.5f, 0.0f});
	}
#endif

	// Calculate pivot location.

	const auto PivotOffset{CalculatePivotOffset()};

	PivotLocation = PivotLagLocation + PivotOffset;

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraShapes)
	{
		DrawDebugLine(GetWorld(), PivotLocation, PivotLagLocation,
		              FLinearColor{0.0f, 0.75f, 1.0f}.ToFColor(true),
		              false, 0.0f, 0, UAlsUtility::DrawLineThickness);

		UAlsUtility::DrawDebugSphereAlternative(GetWorld(), PivotLocation, CameraYawRotation, 16.0f, {0.0f, 0.75f, 1.0f});
	}
#endif

	// Calculate target camera location.

	const auto CameraTargetLocation{PivotLocation + CalculateCameraOffset()};

	// Trace for an object between the camera and character to apply a corrective offset.

	const auto CameraResultLocation{CalculateCameraTrace(CameraTargetLocation, PivotOffset, DeltaTime, bAllowLag)};

	if (PreviousViewMode != Character->GetViewMode())
	{
		// Set aim point correction during change FPP/TPP
		auto FocusLocation{GetCurrentFocusLocation()};
		if (PreviousViewMode == AlsViewModeTags::FirstPerson)
		{
			// FPP -> TPP
			auto TPPCameraLocation{FVector::PointPlaneProject(CameraResultLocation, PivotLocation, -CameraRotation.Vector())};
			auto FocalRotation{(FocusLocation - TPPCameraLocation).Rotation()};
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			if (Settings->HeuristicPitchMapping && IsValid(Settings->HeuristicPitchMapping))
			{
				FocalRotation.Normalize();
				auto Mapped = FMath::Lerp(-180.0, 180.0, Settings->HeuristicPitchMapping->GetFloatValue((FocalRotation.Pitch + 180.0) / 360.0));
				//UE_LOG(LogTemp, Log, TEXT("%.2f -> %.2f"), FocalRotation.Pitch, Mapped);
				FocalRotation.Pitch = Mapped;
			}
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), TPPCameraLocation, FocusLocation, FLinearColor{0.0f, 0.75f, 1.0f}.ToFColor(true),
							  false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		else if (PreviousViewMode == AlsViewModeTags::ThirdPerson)
		{
			// TPP -> FPP
			auto FocalRotation{(FocusLocation - GetEyeCameraLocation()).Rotation()};
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), GetEyeCameraLocation(), FocusLocation, FLinearColor{0.75f, 0.0f, 1.0f}.ToFColor(true),
							  false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		PreviousViewMode = Character->GetViewMode();
	}

	if (bPreviousRightShoulder != bRightShoulder)
	{
		if (Character->GetViewMode() == AlsViewModeTags::ThirdPerson && Character->GetRotationMode() != AlsRotationModeTags::VelocityDirection)
		{
			// Set aim point correction during change shoulder
			auto FocusLocation{GetCurrentFocusLocation()};
			auto CounterpartCameraLocation{FVector::PointPlaneProject(CameraResultLocation, PivotLocation, -CameraRotation.Vector())
				.MirrorByPlane(FPlane(Character->GetActorLocation(), CameraRotation.RotateVector(FVector::RightVector)))};
			auto FocalRotation{(FocusLocation - CounterpartCameraLocation).Rotation()};
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), CounterpartCameraLocation, FocusLocation, FLinearColor{0.0f, 0.75f, 1.0f}.ToFColor(true),
							  false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		bPreviousRightShoulder = bRightShoulder;
	}

	if (bInAutoFPP)
	{
		CalculateAimingFirstPersonCamera(AimingAmount, CameraRotation);
		Camera->FieldOfView = Settings->FirstPerson.FOV;
		Camera->bPanoramic = Settings->FirstPerson.bPanoramic;
		Camera->PanoramicFieldOfView = Settings->FirstPerson.PanoramaFOV;
		Camera->PanoramaSideViewRate = Settings->FirstPerson.PanoramaSideViewRate;

		FocalLength = CalculateFocalLength();
		Character->SetLookRotation(Character->GetViewRotation());
		Camera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
	}
	else if (!FAnimWeight::IsRelevant(FirstPersonOverride))
	{
		CameraLocation = CameraResultLocation;
		Camera->FieldOfView = Settings->ThirdPerson.FOV;
		Camera->bPanoramic = Settings->ThirdPerson.bPanoramic;
		Camera->PanoramicFieldOfView = Settings->ThirdPerson.PanoramaFOV;
		Camera->PanoramaSideViewRate = Settings->ThirdPerson.PanoramaSideViewRate;

		FocalLength = CalculateFocalLength();
		Character->SetLookRotation((GetCurrentFocusLocation() - GetEyeCameraLocation()).Rotation());
		Camera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
	}
	else
	{
		auto FirstPersonCameraLocation{GetFirstPersonCameraLocation() - GetForwardVector() * Settings->FirstPerson.HeadSize};
		CameraLocation = FMath::Lerp(CameraResultLocation, FirstPersonCameraLocation, FirstPersonOverride);
		Camera->FieldOfView = FMath::Lerp(Settings->ThirdPerson.FOV, Settings->FirstPerson.FOV, FirstPersonOverride);
		Camera->bPanoramic = Settings->ThirdPerson.bPanoramic | Settings->FirstPerson.bPanoramic;
		Camera->PanoramicFieldOfView = FMath::Lerp(Settings->ThirdPerson.PanoramaFOV, Settings->FirstPerson.PanoramaFOV, FirstPersonOverride);
		Camera->PanoramaSideViewRate = FMath::Lerp(Settings->ThirdPerson.PanoramaSideViewRate, Settings->FirstPerson.PanoramaSideViewRate, FirstPersonOverride);
		Camera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
	}

	if (bFPP != bInAutoFPP)
	{
		bFPP = bInAutoFPP;
		// call Begin/End FPP Event
		Character->OnChangedPerspective(bFPP);
	}
}

FRotator UAlsCameraSkeletalMeshComponent::CalculateCameraRotation(const FRotator& CameraTargetRotation,
																  const float DeltaTime, const bool bAllowLag) const
{
	if (!bAllowLag)
	{
		return CameraTargetRotation;
	}

	const auto RotationLag{GetAnimInstance()->GetCurveValue(UAlsCameraConstants::RotationLagCurveName())};

	if (!Settings->bEnableCameraLagSubstepping ||
	    DeltaTime <= Settings->CameraLagSubstepping.LagSubstepDeltaTime ||
	    RotationLag <= 0.0f)
	{
		return UAlsMath::ExponentialDecay(CameraRotation, CameraTargetRotation, DeltaTime, RotationLag);
	}

	const auto CameraInitialRotation{CameraRotation};
	const auto SubstepRotationSpeed{(CameraTargetRotation - CameraInitialRotation).GetNormalized() * (1.0f / DeltaTime)};

	auto NewCameraRotation{CameraRotation};
	auto PreviousSubstepTime{0.0f};

	for (auto SubstepNumber{1};; SubstepNumber++)
	{
		const auto SubstepTime{SubstepNumber * Settings->CameraLagSubstepping.LagSubstepDeltaTime};
		if (SubstepTime < DeltaTime - UE_SMALL_NUMBER)
		{
			NewCameraRotation = FMath::RInterpTo(NewCameraRotation, CameraInitialRotation + SubstepRotationSpeed * SubstepTime,
			                                     SubstepTime - PreviousSubstepTime, RotationLag);

			PreviousSubstepTime = SubstepTime;
		}
		else
		{
			return FMath::RInterpTo(NewCameraRotation, CameraTargetRotation, DeltaTime - PreviousSubstepTime, RotationLag);
		}
	}
}

FVector UAlsCameraSkeletalMeshComponent::CalculatePivotLagLocation(const FQuat& CameraYawRotation, const float DeltaTime, const bool bAllowLag) const
{
	if (!bAllowLag)
	{
		return PivotTargetLocation;
	}

	const auto RelativePivotInitialLagLocation{CameraYawRotation.UnrotateVector(PivotLagLocation)};
	const auto RelativePivotTargetLocation{CameraYawRotation.UnrotateVector(PivotTargetLocation)};

	const auto LocationLagX{GetAnimInstance()->GetCurveValue(UAlsCameraConstants::LocationLagXCurveName())};
	const auto LocationLagY{GetAnimInstance()->GetCurveValue(UAlsCameraConstants::LocationLagYCurveName())};
	const auto LocationLagZ{GetAnimInstance()->GetCurveValue(UAlsCameraConstants::LocationLagZCurveName())};

	if (!Settings->bEnableCameraLagSubstepping ||
	    DeltaTime <= Settings->CameraLagSubstepping.LagSubstepDeltaTime ||
	    (LocationLagX <= 0.0f && LocationLagY <= 0.0f && LocationLagZ <= 0.0f))
	{
		return CameraYawRotation.RotateVector({
			UAlsMath::ExponentialDecay(RelativePivotInitialLagLocation.X, RelativePivotTargetLocation.X, DeltaTime, LocationLagX),
			UAlsMath::ExponentialDecay(RelativePivotInitialLagLocation.Y, RelativePivotTargetLocation.Y, DeltaTime, LocationLagY),
			UAlsMath::ExponentialDecay(RelativePivotInitialLagLocation.Z, RelativePivotTargetLocation.Z, DeltaTime, LocationLagZ)
		});
	}

	const auto SubstepMovementSpeed{(RelativePivotTargetLocation - RelativePivotInitialLagLocation) / DeltaTime};

	auto RelativePivotLagLocation{RelativePivotInitialLagLocation};
	auto PreviousSubstepTime{0.0f};

	for (auto SubstepNumber{1};; SubstepNumber++)
	{
		const auto SubstepTime{SubstepNumber * Settings->CameraLagSubstepping.LagSubstepDeltaTime};
		if (SubstepTime < DeltaTime - UE_SMALL_NUMBER)
		{
			const auto SubstepRelativePivotTargetLocation{RelativePivotInitialLagLocation + SubstepMovementSpeed * SubstepTime};
			const auto SubstepDeltaTime{SubstepTime - PreviousSubstepTime};

			RelativePivotLagLocation.X = FMath::FInterpTo(RelativePivotLagLocation.X, SubstepRelativePivotTargetLocation.X,
			                                              SubstepDeltaTime, LocationLagX);
			RelativePivotLagLocation.Y = FMath::FInterpTo(RelativePivotLagLocation.Y, SubstepRelativePivotTargetLocation.Y,
			                                              SubstepDeltaTime, LocationLagY);
			RelativePivotLagLocation.Z = FMath::FInterpTo(RelativePivotLagLocation.Z, SubstepRelativePivotTargetLocation.Z,
			                                              SubstepDeltaTime, LocationLagZ);

			PreviousSubstepTime = SubstepTime;
		}
		else
		{
			const auto RemainingDeltaTime{DeltaTime - PreviousSubstepTime};

			RelativePivotLagLocation.X = FMath::FInterpTo(RelativePivotLagLocation.X, RelativePivotTargetLocation.X,
			                                              RemainingDeltaTime, LocationLagX);
			RelativePivotLagLocation.Y = FMath::FInterpTo(RelativePivotLagLocation.Y, RelativePivotTargetLocation.Y,
			                                              RemainingDeltaTime, LocationLagY);
			RelativePivotLagLocation.Z = FMath::FInterpTo(RelativePivotLagLocation.Z, RelativePivotTargetLocation.Z,
			                                              RemainingDeltaTime, LocationLagZ);

			return CameraYawRotation.RotateVector(RelativePivotLagLocation);
		}
	}
}

FVector UAlsCameraSkeletalMeshComponent::CalculatePivotOffset() const
{
	return Character->GetMesh()->GetComponentQuat().RotateVector(
		FVector{
			GetAnimInstance()->GetCurveValue(UAlsCameraConstants::PivotOffsetXCurveName()),
			GetAnimInstance()->GetCurveValue(UAlsCameraConstants::PivotOffsetYCurveName()),
			GetAnimInstance()->GetCurveValue(UAlsCameraConstants::PivotOffsetZCurveName())
		} * Character->GetMesh()->GetComponentScale().Z);
}

FVector UAlsCameraSkeletalMeshComponent::CalculateCameraOffset() const
{
	return CameraRotation.RotateVector(
		FVector{
			GetAnimInstance()->GetCurveValue(UAlsCameraConstants::CameraOffsetXCurveName()),
			GetAnimInstance()->GetCurveValue(UAlsCameraConstants::CameraOffsetYCurveName()),
			GetAnimInstance()->GetCurveValue(UAlsCameraConstants::CameraOffsetZCurveName())
		} * Character->GetMesh()->GetComponentScale().Z);
}

FVector UAlsCameraSkeletalMeshComponent::CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, const float DeltaTime, const bool bAllowLag)
{
#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebugCameraTraces{
		UAlsUtility::ShouldDisplayDebugForActor(GetOwner(), UAlsCameraConstants::CameraTracesDebugDisplayName())
	};
#endif

	const auto MeshScale{Character->GetMesh()->GetComponentScale().Z};

	static const FName MainTraceTag{FString::Printf(TEXT("%hs (Main Trace)"), __FUNCTION__)};

	auto TraceStart{
		FMath::Lerp(
			GetThirdPersonTraceStartLocation(),
			PivotTargetLocation + PivotOffset + FVector{Settings->ThirdPerson.TraceOverrideOffset},
			UAlsMath::Clamp01(GetAnimInstance()->GetCurveValue(UAlsCameraConstants::TraceOverrideCurveName())))
	};

	const FVector TraceEnd{CameraTargetLocation};
	const auto CollisionShape{FCollisionShape::MakeSphere(Settings->ThirdPerson.TraceRadius * MeshScale)};

	auto TraceResult{TraceEnd};

	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
	                                     CollisionShape, {MainTraceTag, false, GetOwner()}))
	{
		if (!Hit.bStartPenetrating)
		{
			TraceResult = Hit.Location;
		}
		else if (TryAdjustLocationBlockedByGeometry(TraceStart, bDisplayDebugCameraTraces))
		{
			static const FName AdjustedTraceTag{FString::Printf(TEXT("%hs (Adjusted Trace)"), __FUNCTION__)};

			GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
			                                 CollisionShape, {AdjustedTraceTag, false, GetOwner()});
			if (Hit.IsValidBlockingHit())
			{
				TraceResult = Hit.Location;
			}
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraTraces)
	{
		UAlsUtility::DrawDebugSweepSphere(GetWorld(), TraceStart, TraceResult, Settings->ThirdPerson.TraceRadius * MeshScale,
		                                  Hit.IsValidBlockingHit() ? FLinearColor::Red : FLinearColor::Green);
	}
#endif

	// Auto FPP processing

	if (bInAutoFPP || Settings->ThirdPerson.AutoFPPStartDistance > 0.0f)
	{
		auto Distance{FVector::Dist(TraceStart, TraceResult)};
		if (bInAutoFPP)
		{
			if (Distance > Settings->ThirdPerson.AutoFPPEndDistance)
			{
				bInAutoFPP = false;
				TraceDistanceRatio = Settings->ThirdPerson.AutoFPPStartDistance / FVector::Dist(TraceStart, TraceEnd);
			}
			else
			{
				TraceDistanceRatio = 1.0f;
				return GetFirstPersonCameraLocation();
			}
		}
		else
		{
			if (Distance < Settings->ThirdPerson.AutoFPPStartDistance)
			{
				TraceDistanceRatio = 1.0f;
				bInAutoFPP = true;
				return GetFirstPersonCameraLocation();
			}
		}
	}

	// Apply trace distance smoothing.

	if (!bAllowLag || !Settings->ThirdPerson.bEnableTraceDistanceSmoothing)
	{
		TraceDistanceRatio = 1.0f;
		return TraceResult;
	}

	const auto TraceVector{TraceEnd - TraceStart};
	const auto TraceDistance{TraceVector.Size()};

	if (TraceDistance <= UE_KINDA_SMALL_NUMBER)
	{
		TraceDistanceRatio = 1.0f;
		return TraceResult;
	}

	const auto TargetTraceDistanceRatio{UE_REAL_TO_FLOAT(FVector::Dist(TraceStart, TraceResult) / TraceDistance)};

	TraceDistanceRatio = TargetTraceDistanceRatio <= TraceDistanceRatio
					   ? TargetTraceDistanceRatio
					   : UAlsMath::ExponentialDecay(TraceDistanceRatio, TargetTraceDistanceRatio, DeltaTime,
													Settings->ThirdPerson.TraceDistanceSmoothing.InterpolationSpeed);

	return TraceStart + TraceVector * TraceDistanceRatio;
}

bool UAlsCameraSkeletalMeshComponent::TryAdjustLocationBlockedByGeometry(FVector& Location, const bool bDisplayDebugCameraTraces) const
{
	// Based on ComponentEncroachesBlockingGeometry_WithAdjustment().

	const auto MeshScale{Character->GetMesh()->GetComponentScale().Z};
	const auto CollisionShape{FCollisionShape::MakeSphere((Settings->ThirdPerson.TraceRadius + 1.0f) * MeshScale)};

	check(Overlaps.IsEmpty())

	ON_SCOPE_EXIT
	{
		Overlaps.Reset();
	};

	static const FName OverlapMultiTraceTag{FString::Printf(TEXT("%hs (Overlap Multi)"), __FUNCTION__)};

	if (!GetWorld()->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
	                                       CollisionShape, {OverlapMultiTraceTag, false, GetOwner()}))
	{
		return false;
	}

	auto Adjustment{FVector::ZeroVector};
	auto bAnyValidBlock{false};

	FMTDResult MtdResult;

	for (const auto& Overlap : Overlaps)
	{
		if (!Overlap.Component.IsValid() ||
		    Overlap.Component->GetCollisionResponseToChannel(Settings->ThirdPerson.TraceChannel) != ECR_Block)
		{
			continue;
		}

		const auto* OverlapBody{Overlap.Component->GetBodyInstance(NAME_None, true, Overlap.ItemIndex)};

		if (OverlapBody == nullptr || !OverlapBody->OverlapTest(Location, FQuat::Identity, CollisionShape, &MtdResult))
		{
			return false;
		}

		if (!FMath::IsNearlyZero(MtdResult.Distance))
		{
			Adjustment += MtdResult.Direction * MtdResult.Distance;
			bAnyValidBlock = true;
		}
	}

	if (!bAnyValidBlock)
	{
		return false;
	}

	auto AdjustmentDirection{Adjustment};

	if (!AdjustmentDirection.Normalize() ||
	    ((GetOwner()->GetActorLocation() - Location).GetSafeNormal() | AdjustmentDirection) < -UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraTraces)
	{
		DrawDebugLine(GetWorld(), Location, Location + Adjustment,
		              FLinearColor{0.0f, 0.75f, 1.0f}.ToFColor(true),
		              false, 5.0f, 0, UAlsUtility::DrawLineThickness);
	}
#endif

	Location += Adjustment;

	static const FName FreeSpaceTraceTag{FString::Printf(TEXT("%hs (Free Space Overlap)"), __FUNCTION__)};

	return !GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
	                                                 FCollisionShape::MakeSphere(Settings->ThirdPerson.TraceRadius * MeshScale),
	                                                 {FreeSpaceTraceTag, false, GetOwner()});
}

float UAlsCameraSkeletalMeshComponent::CalculateFocalLength() const
{
#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebugTraces{
		UAlsUtility::ShouldDisplayDebugForActor(GetOwner(), UAlsCameraConstants::CameraTracesDebugDisplayName())
	};
#endif
	static const FName MainTraceTag{FString::Printf(TEXT("%hs (Main Trace)"), __FUNCTION__)};
	const auto CollisionShape{FCollisionShape::MakeSphere(Settings->FocusTraceRadius)};

	auto EyeVec = CameraRotation.Vector();
	FVector TraceStart = CameraLocation + EyeVec * Settings->MinFocalLength;
	if (!IsFirstPerson())
	{
		auto ProjectedLocation = FVector::PointPlaneProject(CameraLocation, GetFirstPersonCameraLocation(), -EyeVec);
		auto ProjectedLocationDistance = FVector::Distance(TraceStart, ProjectedLocation);
		TraceStart = ProjectedLocation + EyeVec * FMath::Max(-ProjectedLocationDistance, Settings->ThirdPerson.FocusTraceStartOffset);
	}
	FVector TraceEnd{CameraLocation + EyeVec * Settings->MaxFocalLength};
	FVector TraceResult{TraceEnd};

	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->FocusTraceChannel,
	                                     CollisionShape, {MainTraceTag, false, GetOwner()}))
	{
		TraceResult = Hit.Location;
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugTraces)
	{
		UAlsUtility::DrawDebugSweepSphere(GetWorld(), TraceStart, TraceResult, CollisionShape.GetSphereRadius(),
			Hit.IsValidBlockingHit() ? FLinearColor::Red : FLinearColor::Green);
	}
#endif

	return FMath::Max(Settings->MinFocalLength, FVector::Distance(TraceResult, CameraLocation));
}

void UAlsCameraSkeletalMeshComponent::UpdateADSCameraShake(float FirstPersonOverride, float AimingAmount)
{
	auto GetCameraManager = [this]() {
		const auto* PlayerController{Character.IsValid() ? Cast<APlayerController>(Character->GetController()) : nullptr};
		return PlayerController && IsValid(PlayerController) ? PlayerController->PlayerCameraManager.Get() : nullptr;
	};

	if (!CurrentADSCameraShake && AimingAmount > Settings->FirstPerson.ADSThreshold && IsValid(ADSCameraShakeClass) &&
		Character->HasSight() && (FAnimWeight::IsFullWeight(FirstPersonOverride) || bInAutoFPP))
	{
		auto* CameraManager{GetCameraManager()};
		if (CameraManager)
		{
			CurrentADSCameraShake = CameraManager->StartCameraShake(ADSCameraShakeClass, ADSCameraShakeScale);
		}
	}
	else if(CurrentADSCameraShake && AimingAmount < Settings->FirstPerson.ADSThreshold)
	{
		auto* CameraManager{GetCameraManager()};
		if (CameraManager)
		{
			CameraManager->StopCameraShake(CurrentADSCameraShake);
			CurrentADSCameraShake = nullptr;
		}
	}
}

bool UAlsCameraSkeletalMeshComponent::IsFirstPerson() const
{
	return FAnimWeight::IsFullWeight(UAlsMath::Clamp01(GetAnimInstance()->GetCurveValue(UAlsCameraConstants::FirstPersonOverrideCurveName())))
		|| bInAutoFPP;
}
