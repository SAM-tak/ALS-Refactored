#include "AlsCharacter.h"

#include "AlsAnimationInstance.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/NetConnection.h"
#include "Engine/SkeletalMesh.h"
#include "Net/Core/PushModel/PushModel.h"
#include "RootMotionSources/AlsRootMotionSource_Mantling.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsLog.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"

bool AAlsCharacter::StartMantlingGrounded()
{
	return GetLocomotionMode() == AlsLocomotionModeTags::Grounded &&
	       StartMantling(Settings->Mantling.GroundedTrace);
}

bool AAlsCharacter::StartMantlingInAir()
{
	return GetLocomotionMode() == AlsLocomotionModeTags::InAir && IsLocallyControlled() &&
	       StartMantling(Settings->Mantling.InAirTrace);
}

bool AAlsCharacter::IsMantlingAllowedToStart_Implementation() const
{
	return !GetLocomotionAction().IsValid();
}

bool AAlsCharacter::StartMantling(const FAlsMantlingTraceSettings& TraceSettings)
{
	if (!Settings->Mantling.bAllowMantling || GetLocalRole() <= ROLE_SimulatedProxy || !IsMantlingAllowedToStart())
	{
		return false;
	}

	const auto ActorLocation{GetActorLocation()};
	const auto ActorYawAngle{UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(GetActorRotation().Yaw))};

	float ForwardTraceAngle;
	if (LocomotionState.bHasSpeed)
	{
		ForwardTraceAngle = LocomotionState.bHasInput
			                    ? LocomotionState.VelocityYawAngle +
			                      FMath::ClampAngle(LocomotionState.InputYawAngle - LocomotionState.VelocityYawAngle,
			                                        -Settings->Mantling.MaxReachAngle, Settings->Mantling.MaxReachAngle)
			                    : LocomotionState.VelocityYawAngle;
	}
	else
	{
		ForwardTraceAngle = LocomotionState.bHasInput ? LocomotionState.InputYawAngle : ActorYawAngle;
	}

	const auto ForwardTraceDeltaAngle{FRotator3f::NormalizeAxis(ForwardTraceAngle - ActorYawAngle)};
	if (FMath::Abs(ForwardTraceDeltaAngle) > Settings->Mantling.TraceAngleThreshold)
	{
		return false;
	}

	const auto ForwardTraceDirection{
		UAlsMath::AngleToDirectionXY(
			ActorYawAngle + FMath::ClampAngle(ForwardTraceDeltaAngle, -Settings->Mantling.MaxReachAngle, Settings->Mantling.MaxReachAngle))
	};

#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebug{UAlsUtility::ShouldDisplayDebugForActor(this, UAlsConstants::MantlingDebugDisplayName())};
#endif

	const auto* Capsule{GetCapsuleComponent()};

	const auto CapsuleScale{Capsule->GetComponentScale().Z};
	const auto CapsuleRadius{Capsule->GetScaledCapsuleRadius()};
	const auto CapsuleHalfHeight{Capsule->GetScaledCapsuleHalfHeight()};

	const FVector CapsuleBottomLocation{ActorLocation.X, ActorLocation.Y, ActorLocation.Z - CapsuleHalfHeight};

	const auto TraceCapsuleRadius{CapsuleRadius - 1.0f};

	const auto LedgeHeightDelta{UE_REAL_TO_FLOAT((TraceSettings.LedgeHeight.GetMax() - TraceSettings.LedgeHeight.GetMin()) * CapsuleScale)};

	// Trace forward to find an object the character cannot walk on.

	static const FName ForwardTraceTag{FString::Printf(TEXT("%hs (Forward Trace)"), __FUNCTION__)};

	auto ForwardTraceStart{CapsuleBottomLocation - ForwardTraceDirection * CapsuleRadius};
	ForwardTraceStart.Z += (TraceSettings.LedgeHeight.X + TraceSettings.LedgeHeight.Y) *
		0.5f * CapsuleScale - UCharacterMovementComponent::MAX_FLOOR_DIST;

	auto ForwardTraceEnd{ForwardTraceStart + ForwardTraceDirection * (CapsuleRadius + (TraceSettings.ReachDistance + 1.0f) * CapsuleScale)};

	const auto ForwardTraceCapsuleHalfHeight{LedgeHeightDelta * 0.5f};

	FHitResult ForwardTraceHit;
	GetWorld()->SweepSingleByChannel(ForwardTraceHit, ForwardTraceStart, ForwardTraceEnd,
	                                 FQuat::Identity, Settings->Mantling.MantlingTraceChannel,
	                                 FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight),
	                                 {ForwardTraceTag, false, this}, Settings->Mantling.MantlingTraceResponses);

	auto* TargetPrimitive{ForwardTraceHit.GetComponent()};

	if (!ForwardTraceHit.IsValidBlockingHit() ||
	    !IsValid(TargetPrimitive) ||
	    TargetPrimitive->GetComponentVelocity().SizeSquared() > FMath::Square(Settings->Mantling.TargetPrimitiveSpeedThreshold) ||
	    !TargetPrimitive->CanCharacterStepUp(this) ||
	    GetCharacterMovement()->IsWalkable(ForwardTraceHit))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(GetWorld(), ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, false, ForwardTraceHit, {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);
		}
#endif

		return false;
	}

	const auto TargetDirection{-ForwardTraceHit.ImpactNormal.GetSafeNormal2D()};

	// Trace downward from the first trace's impact point and determine if the hit location is walkable.

	static const FName DownwardTraceTag{FString::Printf(TEXT("%hs (Downward Trace)"), __FUNCTION__)};

	const FVector2D TargetLocationOffset{TargetDirection * (TraceSettings.TargetLocationOffset * CapsuleScale)};

	const FVector DownwardTraceStart{
		ForwardTraceHit.ImpactPoint.X + TargetLocationOffset.X,
		ForwardTraceHit.ImpactPoint.Y + TargetLocationOffset.Y,
		CapsuleBottomLocation.Z + LedgeHeightDelta + 2.5f * TraceCapsuleRadius + UCharacterMovementComponent::MIN_FLOOR_DIST
	};

	const FVector DownwardTraceEnd{
		DownwardTraceStart.X,
		DownwardTraceStart.Y,
		CapsuleBottomLocation.Z +
		TraceSettings.LedgeHeight.GetMin() * CapsuleScale + TraceCapsuleRadius - UCharacterMovementComponent::MAX_FLOOR_DIST
	};

	FHitResult DownwardTraceHit;
	GetWorld()->SweepSingleByChannel(DownwardTraceHit, DownwardTraceStart, DownwardTraceEnd, FQuat::Identity,
	                                 Settings->Mantling.MantlingTraceChannel, FCollisionShape::MakeSphere(TraceCapsuleRadius),
	                                 {DownwardTraceTag, false, this}, Settings->Mantling.MantlingTraceResponses);

	const auto SlopeAngleCos{UE_REAL_TO_FLOAT(DownwardTraceHit.ImpactNormal.Z)};

	// The approximate slope angle is used in situations where the normal slope angle cannot convey
	// the true nature of the surface slope, for example, for a 45 degree staircase the slope
	// angle will always be 90 degrees, while the approximate slope angle will be ~45 degrees.

	auto ApproximateSlopeNormal{DownwardTraceHit.Location - DownwardTraceHit.ImpactPoint};
	ApproximateSlopeNormal.Normalize();

	const auto ApproximateSlopeAngleCos{UE_REAL_TO_FLOAT(ApproximateSlopeNormal.Z)};

	if (SlopeAngleCos < Settings->Mantling.SlopeAngleThresholdCos ||
	    ApproximateSlopeAngleCos < Settings->Mantling.SlopeAngleThresholdCos ||
	    !GetCharacterMovement()->IsWalkable(DownwardTraceHit))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(GetWorld(), ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit, {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);

			UAlsUtility::DrawDebugSweepSingleSphere(GetWorld(), DownwardTraceStart, DownwardTraceEnd, TraceCapsuleRadius,
			                                        false, DownwardTraceHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                        TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);
		}
#endif

		return false;
	}

	// Check that there is enough free space for the capsule at the target location.

	static const FName TargetLocationTraceTag{FString::Printf(TEXT("%hs (Target Location Overlap)"), __FUNCTION__)};

	const FVector TargetLocation{
		DownwardTraceHit.Location.X,
		DownwardTraceHit.Location.Y,
		DownwardTraceHit.ImpactPoint.Z + UCharacterMovementComponent::MIN_FLOOR_DIST
	};

	const FVector TargetCapsuleLocation{TargetLocation.X, TargetLocation.Y, TargetLocation.Z + CapsuleHalfHeight};

	if (GetWorld()->OverlapBlockingTestByChannel(TargetCapsuleLocation, FQuat::Identity, Settings->Mantling.MantlingTraceChannel,
	                                             FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
	                                             {TargetLocationTraceTag, false, this}, Settings->Mantling.MantlingTraceResponses))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(GetWorld(), ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit, {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);

			UAlsUtility::DrawDebugSweepSingleSphere(GetWorld(), DownwardTraceStart, DownwardTraceEnd, TraceCapsuleRadius,
			                                        false, DownwardTraceHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                        TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);

			DrawDebugCapsule(GetWorld(), TargetCapsuleLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity,
			                 FColor::Red, false, TraceSettings.bDrawFailedTraces ? 10.0f : 0.0f);
		}
#endif

		return false;
	}

	// Perform additional overlap at the approximate start location to
	// ensure there are no vertical obstacles on the path, such as a ceiling.

	static const FName StartLocationTraceTag{FString::Printf(TEXT("%hs (Start Location Overlap)"), __FUNCTION__)};

	const FVector2D StartLocationOffset{TargetDirection * (TraceSettings.StartLocationOffset * CapsuleScale)};

	const FVector StartLocation{
		ForwardTraceHit.ImpactPoint.X - StartLocationOffset.X,
		ForwardTraceHit.ImpactPoint.Y - StartLocationOffset.Y,
		(DownwardTraceHit.Location.Z + DownwardTraceEnd.Z) * 0.5f
	};

	const auto StartLocationTraceCapsuleHalfHeight{(DownwardTraceHit.Location.Z - DownwardTraceEnd.Z) * 0.5f + TraceCapsuleRadius};

	if (GetWorld()->OverlapBlockingTestByChannel(StartLocation, FQuat::Identity, Settings->Mantling.MantlingTraceChannel,
	                                             FCollisionShape::MakeCapsule(TraceCapsuleRadius, StartLocationTraceCapsuleHalfHeight),
	                                             {StartLocationTraceTag, false, this}, Settings->Mantling.MantlingTraceResponses))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(GetWorld(), ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit,
			                                                    {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);

			UAlsUtility::DrawDebugSweepSingleSphere(GetWorld(), DownwardTraceStart, DownwardTraceEnd, TraceCapsuleRadius,
			                                        false, DownwardTraceHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                        TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);

			DrawDebugCapsule(GetWorld(), StartLocation, StartLocationTraceCapsuleHalfHeight, TraceCapsuleRadius, FQuat::Identity,
			                 FLinearColor{1.0f, 0.5f, 0.0f}.ToFColor(true), false, TraceSettings.bDrawFailedTraces ? 10.0f : 0.0f);
		}
#endif

		return false;
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebug)
	{
		UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(GetWorld(), ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
		                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit,
		                                                    {0.0f, 0.25f, 1.0f}, {0.0f, 0.75f, 1.0f}, 5.0f);

		UAlsUtility::DrawDebugSweepSingleSphere(GetWorld(), DownwardTraceStart, DownwardTraceEnd,
		                                        TraceCapsuleRadius, true, DownwardTraceHit,
		                                        {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f}, 7.5f);
	}
#endif

	const auto TargetRotation{TargetDirection.ToOrientationQuat()};

	FAlsMantlingParameters Parameters;

	Parameters.TargetPrimitive = TargetPrimitive;
	Parameters.MantlingHeight = UE_REAL_TO_FLOAT((TargetLocation.Z - CapsuleBottomLocation.Z) / CapsuleScale);

	// Determine the mantling type by checking the movement mode and mantling height.

	Parameters.MantlingType = GetLocomotionMode() != AlsLocomotionModeTags::Grounded
		                          ? EAlsMantlingType::InAir
		                          : Parameters.MantlingHeight > Settings->Mantling.MantlingHighHeightThreshold
		                          ? EAlsMantlingType::High
		                          : EAlsMantlingType::Low;

	// If the target primitive can't move, then use world coordinates to save
	// some performance by skipping some coordinate space transformations later.

	if (MovementBaseUtility::UseRelativeLocation(TargetPrimitive))
	{
		const auto TargetRelativeTransform{
			TargetPrimitive->GetComponentTransform().GetRelativeTransform({TargetRotation, TargetLocation})
		};

		Parameters.TargetRelativeLocation = TargetRelativeTransform.GetLocation();
		Parameters.TargetRelativeRotation = TargetRelativeTransform.Rotator();
	}
	else
	{
		Parameters.TargetRelativeLocation = TargetLocation;
		Parameters.TargetRelativeRotation = TargetRotation.Rotator();
	}

	if (GetLocalRole() >= ROLE_Authority)
	{
		MulticastStartMantling(Parameters);
	}
	else
	{
		GetCharacterMovement()->FlushServerMoves();

		StartMantlingImplementation(Parameters);
		ServerStartMantling(Parameters);
	}

	return true;
}

void AAlsCharacter::ServerStartMantling_Implementation(const FAlsMantlingParameters& Parameters)
{
	if (IsMantlingAllowedToStart())
	{
		MulticastStartMantling(Parameters);
		ForceNetUpdate();
	}
}

void AAlsCharacter::MulticastStartMantling_Implementation(const FAlsMantlingParameters& Parameters)
{
	StartMantlingImplementation(Parameters);
}

void AAlsCharacter::StartMantlingImplementation(const FAlsMantlingParameters& Parameters)
{
	if (!IsMantlingAllowedToStart())
	{
		return;
	}

	const auto* MantlingSettings{SelectMantlingSettings(Parameters.MantlingType)};

	if (!ALS_ENSURE(IsValid(MantlingSettings)) || !ALS_ENSURE(IsValid(MantlingSettings->Montage)))
	{
		return;
	}

	const auto StartTime{CalculateMantlingStartTime(MantlingSettings, Parameters.MantlingHeight)};
	const auto Duration{MantlingSettings->Montage->GetPlayLength() - StartTime};
	const auto PlayRate{MantlingSettings->Montage->RateScale};

	const auto TargetAnimationLocation{UAlsUtility::ExtractLastRootTransformFromMontage(MantlingSettings->Montage).GetLocation()};

	if (FMath::IsNearlyZero(TargetAnimationLocation.Z))
	{
		UE_LOG(LogAls, Warning, TEXT("Can't start mantling! The %s animation montage has incorrect root motion,")
		       TEXT(" the final vertical location of the character must be non-zero!"), *MantlingSettings->Montage->GetName());
		return;
	}

	// Calculate actor offsets (offsets between actor and target transform).

	const auto bUseRelativeLocation{MovementBaseUtility::UseRelativeLocation(Parameters.TargetPrimitive.Get())};
	const auto TargetRelativeRotation{Parameters.TargetRelativeRotation.GetNormalized()};

	const auto TargetTransform{
		bUseRelativeLocation
			? FTransform{
				TargetRelativeRotation, Parameters.TargetRelativeLocation,
				Parameters.TargetPrimitive->GetComponentScale()
			}.GetRelativeTransformReverse(Parameters.TargetPrimitive->GetComponentTransform())
			: FTransform{TargetRelativeRotation, Parameters.TargetRelativeLocation}
	};

	const auto ActorFeetLocationOffset{GetCharacterMovement()->GetActorFeetLocation() - TargetTransform.GetLocation()};
	const auto ActorRotationOffset{TargetTransform.GetRotation().Inverse() * GetActorQuat()};

	// Reset network smoothing.

	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;

	GetMesh()->SetRelativeLocationAndRotation(GetBaseTranslationOffset(),
	                                          GetMesh()->IsUsingAbsoluteRotation()
		                                          ? GetActorQuat() * GetBaseRotationOffset()
		                                          : GetBaseRotationOffset(), false, nullptr, ETeleportType::TeleportPhysics);

	// Clear the character movement mode and set the locomotion action to mantling.

	GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	AlsCharacterMovement->SetMovementModeLocked(true);

	GetCharacterMovement()->SetBase(Parameters.TargetPrimitive.Get());

	// Apply mantling root motion.

	const auto RootMotionSource{MakeShared<FAlsRootMotionSource_Mantling>()};
	RootMotionSource->InstanceName = __FUNCTION__;
	RootMotionSource->Duration = Duration / PlayRate;
	RootMotionSource->MantlingSettings = MantlingSettings;
	RootMotionSource->TargetPrimitive = Parameters.TargetPrimitive;
	RootMotionSource->TargetRelativeLocation = Parameters.TargetRelativeLocation;
	RootMotionSource->TargetRelativeRotation = TargetRelativeRotation;
	RootMotionSource->ActorFeetLocationOffset = ActorFeetLocationOffset;
	RootMotionSource->ActorRotationOffset = ActorRotationOffset.Rotator();
	RootMotionSource->TargetAnimationLocation = TargetAnimationLocation;
	RootMotionSource->MontageStartTime = StartTime;

	MantlingState.RootMotionSourceId = GetCharacterMovement()->ApplyRootMotionSource(RootMotionSource);

	// Play the animation montage if valid.

	if (GetMesh()->GetAnimInstance()->Montage_Play(MantlingSettings->Montage, 1.0f,
	                                               EMontagePlayReturnType::MontageLength,
	                                               StartTime, false))
	{
		SetLocomotionAction(AlsLocomotionActionTags::Mantling);
	}

	OnMantlingStarted(Parameters);
}

UAlsMantlingSettings* AAlsCharacter::SelectMantlingSettings_Implementation(EAlsMantlingType MantlingType)
{
	return nullptr;
}

float AAlsCharacter::CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, const float MantlingHeight) const
{
	if (!MantlingSettings->bAutoCalculateStartTime)
	{
		return FMath::GetMappedRangeValueClamped(MantlingSettings->StartTimeReferenceHeight, MantlingSettings->StartTime, MantlingHeight);
	}

	// https://landelare.github.io/2022/05/15/climbing-with-root-motion.html

	const auto* Montage{MantlingSettings->Montage.Get()};
	if (!IsValid(Montage))
	{
		return 0.0f;
	}

	const auto MontageFrameRate{1.0f / Montage->GetSamplingFrameRate().AsDecimal()};

	auto SearchStartTime{0.0f};
	auto SearchEndTime{Montage->GetPlayLength()};

	const auto SearchStartLocationZ{UAlsUtility::ExtractRootTransformFromMontage(Montage, SearchStartTime).GetTranslation().Z};
	const auto SearchEndLocationZ{UAlsUtility::ExtractRootTransformFromMontage(Montage, SearchEndTime).GetTranslation().Z};

	// Find the vertical distance the character has already moved.

	const auto TargetLocationZ{FMath::Max(0.0f, SearchEndLocationZ - MantlingHeight)};

	// Perform a binary search to find the time when the character is at the target vertical distance.

	static constexpr auto MaxLocationSearchTolerance{1.0f};

	if (FMath::IsNearlyEqual(SearchStartLocationZ, TargetLocationZ, MaxLocationSearchTolerance))
	{
		return SearchStartTime;
	}

	while (true)
	{
		const auto Time{(SearchStartTime + SearchEndTime) * 0.5f};
		const auto LocationZ{UAlsUtility::ExtractRootTransformFromMontage(Montage, Time).GetTranslation().Z};

		// Stop the search if a close enough location has been found or if
		// the search interval is less than the animation montage frame rate.

		if (FMath::IsNearlyEqual(LocationZ, TargetLocationZ, MaxLocationSearchTolerance) ||
		    SearchEndTime - SearchStartTime <= MontageFrameRate)
		{
			return Time;
		}

		if (LocationZ < TargetLocationZ)
		{
			SearchStartTime = Time;
		}
		else
		{
			SearchEndTime = Time;
		}
	}
}

void AAlsCharacter::OnMantlingStarted_Implementation(const FAlsMantlingParameters& Parameters) {}

void AAlsCharacter::RefreshMantling()
{
	if (MantlingState.RootMotionSourceId <= 0)
	{
		return;
	}

	if (GetLocomotionAction() != AlsLocomotionActionTags::Mantling)
	{
		StopMantling();
		return;
	}

	if (GetCharacterMovement()->MovementMode != MOVE_Custom)
	{
		StopMantling(true);
		return;
	}

	const auto* RootMotionSource{
		StaticCastSharedPtr<FAlsRootMotionSource_Mantling>(GetCharacterMovement()
			->GetRootMotionSourceByID(MantlingState.RootMotionSourceId)).Get()
	};

	if (RootMotionSource != nullptr && !RootMotionSource->TargetPrimitive.IsValid())
	{
		StopMantling(true);

		if (Settings->Mantling.bStartRagdollingOnTargetPrimitiveDestruction)
		{
			AbilitySystem->TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::Ragdolling);
		}
	}
}

void AAlsCharacter::StopMantling(const bool bStopMontage)
{
	if (MantlingState.RootMotionSourceId <= 0)
	{
		return;
	}

	auto* RootMotionSource{
		StaticCastSharedPtr<FAlsRootMotionSource_Mantling>(GetCharacterMovement()
			->GetRootMotionSourceByID(MantlingState.RootMotionSourceId)).Get()
	};

	if (RootMotionSource != nullptr)
	{
		RootMotionSource->Status.SetFlag(ERootMotionSourceStatusFlags::MarkedForRemoval);
	}

	MantlingState.RootMotionSourceId = 0;

	if (bStopMontage && RootMotionSource != nullptr)
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(Settings->Mantling.BlendOutDuration, RootMotionSource->MantlingSettings->Montage);
	}

	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

	AlsCharacterMovement->SetMovementModeLocked(false);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	OnMantlingEnded();

	ForceNetUpdate();
}

void AAlsCharacter::OnMantlingEnded_Implementation() {}
