#include "AlsGameplayCameraState.h"

#include "AlsGameplayCameraStateSettings.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/GameplayCameraComponentBase.h"
#include "Core/CameraSystemEvaluator.h"
#include "Core/CameraVariableAssets.h"
#include "Core/RootCameraNode.h"
#include "Curves/CurveFloat.h"
#include "Engine/OverlapResult.h"
#include "Misc/UObjectToken.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Utility/AlsCameraConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayCameraState)

UAlsGameplayCameraState::UAlsGameplayCameraState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAlsGameplayCameraState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ConfirmedDesiredViewMode, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ShoulderMode, Parameters)
}

void UAlsGameplayCameraState::SetUp(AAlsCharacter* NewCharacter, UGameplayCameraComponentBase* NewGameplayCameraComponent)
{
#if !UE_BUILD_SHIPPING
	if (Character.IsValid())
	{
		Character->OnDisplayDebug.RemoveAll(this);
	}
	if (NewCharacter)
	{
		NewCharacter->OnDisplayDebug.AddUObject(this, &ThisClass::DisplayDebug);
	}
#endif
	Character = NewCharacter;
	GameplayCameraComponent = NewGameplayCameraComponent;
	if (!IsValid(Settings) || !Character.IsValid() || !GameplayCameraComponent.IsValid())
	{
		return;
	}
	PreviousShoulderMode = ShoulderMode = Settings->ThirdPerson.ShoulderMode;
	PreviousConfirmedDesiredViewMode = DesiredViewMode = Settings->DesiredViewMode;
	Character->SetViewMode(DesiredViewMode);
	SetConfirmedDesiredViewMode(DesiredViewMode);
}

void UAlsGameplayCameraState::Tick(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UAlsGameplayCameraState::Tick()"), STAT_UAlsGameplayCameraState_Tick, STATGROUP_Als)

	if (!IsValid(Settings) || !Character.IsValid() || !GameplayCameraComponent.IsValid())
	{
		return;
	}

	FirstPersonFactor = 0.0f;

	auto CameraSystemEvaluator = GameplayCameraComponent->GetCameraSystemEvaluator();
	if (CameraSystemEvaluator.IsValid())
	{
		const auto& Result = CameraSystemEvaluator->GetEvaluatedResult();
		CameraLocation = Result.CameraPose.GetLocation();
		CameraRotation = Result.CameraPose.GetRotation();
		if(FirstPersonFactorVariableId.IsValid())
		{
			float OutValue;
			if (Result.VariableTable.TryGetValue<float>(FirstPersonFactorVariableId, OutValue))
			{
				FirstPersonFactor = UAlsMath::Clamp01(OutValue);
			}
		}
	}

#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebugCameraShapes{
		UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraShapesDebugDisplayName())
	};
	const auto bDisplayDebugCameraTraces{
		UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraTracesDebugDisplayName())
	};
#endif

	// Refresh desired view mode information.

	if (Character->IsCharacterSelf())
	{
		if (ViewModeChangeBlockTime > 0.f)
		{
			ViewModeChangeBlockTime -= DeltaTime;
		}
		else
		{
			if (DesiredViewMode != ConfirmedDesiredViewMode)
			{
				ViewModeChangeBlockTime = Settings->ViewModeChangeBlockTime;
			}
			SetConfirmedDesiredViewMode(DesiredViewMode);
		}
	}

	// Refresh movement base.

	const auto& BasedMovement{ Character->GetBasedMovement() };
	const auto bMovementBaseHasRelativeRotation{ BasedMovement.HasRelativeRotation() };

	auto MovementBaseLocation{ FVector::ZeroVector };
	auto MovementBaseRotation{ FQuat::Identity };

	if (bMovementBaseHasRelativeRotation)
	{
		MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBaseLocation, MovementBaseRotation);
	}

	if (BasedMovement.MovementBase != MovementBasePrimitive || BasedMovement.BoneName != MovementBaseBoneName)
	{
		MovementBasePrimitive = BasedMovement.MovementBase;
		MovementBaseBoneName = BasedMovement.BoneName;

		if (bMovementBaseHasRelativeRotation)
		{
			const auto MovementBaseRotationInverse{ MovementBaseRotation.Inverse() };

			PivotMovementBaseRelativeLagLocation = MovementBaseRotationInverse.RotateVector(CameraLocation - MovementBaseLocation);
			CameraMovementBaseRelativeRotation = MovementBaseRotationInverse * CameraRotation.Quaternion();
		}
		else
		{
			PivotMovementBaseRelativeLagLocation = FVector::ZeroVector;
			CameraMovementBaseRelativeRotation = FQuat::Identity;
		}
	}

	const auto AimingAmount{ Character->GetAimAmount() };

	UpdateADSCameraShake(FirstPersonFactor, AimingAmount);

	//PivotTargetLocation = GetThirdPersonPivotLocation();

	if (FAnimWeight::IsFullWeight(FirstPersonFactor))
	{
		// Skip other calculations if the character is fully in first-person mode.

		ViewMode = ConfirmedDesiredViewMode;
		bInAutoFPP = false;

		UpdateAimingFirstPersonCamera(AimingAmount, CameraRotation);
		UpdateFocalLength();
		Character->SetLookRotation(Character->GetViewRotation());
		Character->SetViewMode(AlsViewModeTags::FirstPerson);
		RefreshTanHalfFov(DeltaTime);
		return;
	}

	// Calculate camera rotation.

	//if (bMovementBaseHasRelativeRotation)
	//{
	//	CameraRotation = (MovementBaseRotation * CameraMovementBaseRelativeRotation).Rotator();

	//	CameraMovementBaseRelativeRotation = MovementBaseRotation.Inverse() * CameraRotation.Quaternion();
	//}
	//else
	//{
	//	CameraRotation = CameraTargetRotation;
	//}

	const FRotator CameraYawRotation{ 0.0f, CameraRotation.Yaw, 0.0f };

//#if ENABLE_DRAW_DEBUG
//	if (bDisplayDebugCameraShapes)
//	{
//		UAlsUtility::DrawDebugSphereAlternative(GetWorld(), PivotTargetLocation, CameraYawRotation, 16.0f, FLinearColor::Green);
//	}
//#endif

	// Calculate pivot lag location. Get the pivot target location and interpolate using axis-independent lag for maximum control.

	//if (bMovementBaseHasRelativeRotation)
	//{
	//	PivotLagLocation = MovementBaseLocation + MovementBaseRotation.RotateVector(PivotMovementBaseRelativeLagLocation);

	//	PivotMovementBaseRelativeLagLocation = MovementBaseRotation.UnrotateVector(PivotLagLocation - MovementBaseLocation);
	//}

//#if ENABLE_DRAW_DEBUG
//	if (bDisplayDebugCameraShapes)
//	{
//		DrawDebugLine(GetWorld(), PivotLagLocation, PivotTargetLocation,
//			FLinearColor{ 1.0f, 0.5f, 0.0f }.ToFColor(true),
//			false, 0.0f, 0, UAlsUtility::DrawLineThickness);
//
//		UAlsUtility::DrawDebugSphereAlternative(GetWorld(), PivotLagLocation, CameraYawRotation, 16.0f, { 1.0f, 0.5f, 0.0f });
//	}
//#endif

	// Calculate target camera location.

	const auto CameraTargetLocation{ CameraLocation };

	// Trace for an object between the camera and character to apply a corrective offset.

	const auto CameraResultLocation{ CalculateCameraTrace(CameraTargetLocation, FVector::Zero(), DeltaTime)};

	if (PreviousConfirmedDesiredViewMode != ConfirmedDesiredViewMode)
	{
		// Set aim point correction during change FPP/TPP
		auto FocusLocation{ GetCurrentFocusLocation() };
		if (PreviousConfirmedDesiredViewMode == AlsCameraViewModeTags::FirstPerson)
		{
			// FPP -> TPP
			auto TPPCameraLocation{ FVector::PointPlaneProject(CameraResultLocation, FVector::Zero(), -CameraRotation.Vector()) };
			auto FocalRotation{ (FocusLocation - TPPCameraLocation).Rotation() };
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
				DrawDebugLine(GetWorld(), TPPCameraLocation, FocusLocation, FLinearColor{ 0.0f, 0.75f, 1.0f }.ToFColor(true),
					false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		else if(PreviousConfirmedDesiredViewMode == AlsCameraViewModeTags::ThirdPerson)
		{
			// TPP -> FPP
			auto FocalRotation{ (FocusLocation - GetEyeCameraLocation()).Rotation() };
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), GetEyeCameraLocation(), FocusLocation, FLinearColor{ 0.75f, 0.0f, 1.0f }.ToFColor(true),
					false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		PreviousConfirmedDesiredViewMode = ConfirmedDesiredViewMode;
	}

	if (PreviousShoulderMode != ShoulderMode)
	{
		if (Character->GetViewMode() == AlsViewModeTags::ThirdPerson && Character->GetRotationMode() != AlsRotationModeTags::VelocityDirection && bIsFocusPawn)
		{
			// Set aim point correction during change shoulder
			auto FocusLocation{ GetCurrentFocusLocation() };
			auto CounterpartCameraLocation{ FVector::PointPlaneProject(CameraResultLocation, FVector::Zero(), -CameraRotation.Vector())
				.MirrorByPlane(FPlane(Character->GetActorLocation(), CameraRotation.RotateVector(FVector::RightVector))) };
			auto FocalRotation{ (FocusLocation - CounterpartCameraLocation).Rotation() };
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), CounterpartCameraLocation, FocusLocation, FLinearColor{ 0.0f, 0.75f, 1.0f }.ToFColor(true),
					false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		PreviousShoulderMode = ShoulderMode;
	}

	if (bInAutoFPP)
	{
		UpdateAimingFirstPersonCamera(AimingAmount, CameraRotation);
		UpdateFocalLength();
		Character->SetLookRotation(Character->GetViewRotation());
	}
	else if(!FAnimWeight::IsRelevant(FirstPersonFactor))
	{
		CameraLocation = CameraResultLocation;

		UpdateFocalLength();
		Character->SetLookRotation((GetCurrentFocusLocation() - GetEyeCameraLocation()).Rotation());
	}
	else
	{
		auto FirstPersonCameraLocation{ GetFirstPersonCameraLocation() - Character->GetMesh()->GetForwardVector() * Settings->FirstPerson.HeadSize };
		CameraLocation = FMath::Lerp(CameraResultLocation, FirstPersonCameraLocation, FirstPersonFactor);
	}

	ViewMode = bInAutoFPP ? AlsCameraViewModeTags::FirstPerson : ConfirmedDesiredViewMode;
	Character->SetViewMode(AlsViewModeTags::ThirdPerson);
	RefreshTanHalfFov(DeltaTime);
}

void UAlsGameplayCameraState::SetFirstPersonFactorVariable(UFloatCameraVariable* FirstPersonFactorVariable)
{
	if (FirstPersonFactorVariable)
	{
		FirstPersonFactorVariableId = FirstPersonFactorVariable->GetVariableID();
	}
}

FVector UAlsGameplayCameraState::CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, const float DeltaTime)
{
#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebugCameraTraces{
		UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraTracesDebugDisplayName())
	};
#endif

	const auto MeshScale{ UE_REAL_TO_FLOAT(Character->GetMesh()->GetComponentScale().Z) };

	static const FName MainTraceTag{ FString::Printf(TEXT("%hs (Main Trace)"), __FUNCTION__) };

	auto TraceStart{ GetThirdPersonTraceStartLocation() };

	const FVector TraceEnd{ CameraTargetLocation };
	const auto CollisionShape{ FCollisionShape::MakeSphere(Settings->ThirdPerson.TraceRadius * MeshScale) };

	auto TraceResult{ TraceEnd };

	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
		CollisionShape, { MainTraceTag, false, Character.Get() }))
	{
		if (!Hit.bStartPenetrating)
		{
			TraceResult = Hit.Location;
		}
		else if(TryAdjustLocationBlockedByGeometry(TraceStart, bDisplayDebugCameraTraces))
		{
			static const FName AdjustedTraceTag{ FString::Printf(TEXT("%hs (Adjusted Trace)"), __FUNCTION__) };

			GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
				CollisionShape, { AdjustedTraceTag, false, Character.Get() });
			if(Hit.IsValidBlockingHit()) {
				TraceResult = Hit.Location;
			}
		}
		else
		{
			// Note that TraceStart may be changed even if TryAdjustLocationBlockedByGeometry() returned false.
			TraceResult = TraceStart;
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraTraces)
	{
		UAlsUtility::DrawDebugSweepSphere(GetWorld(), TraceStart, TraceResult, CollisionShape.GetCapsuleRadius(),
			Hit.IsValidBlockingHit() ? FLinearColor::Red : FLinearColor::Green);
	}
#endif

	// Auto FPP processing

	if (bInAutoFPP || (Settings->ThirdPerson.AutoFPPStartDistance > 0.0f && !Character->GetLocomotionAction().IsValid() && Hit.IsValidBlockingHit()))
	{
		auto Distance{ FVector::Dist(TraceStart, TraceResult) };
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

	if (!Settings->ThirdPerson.bEnableTraceDistanceSmoothing)
	{
		TraceDistanceRatio = 1.0f;
		return TraceResult;
	}

	const auto TraceVector{ TraceEnd - TraceStart };
	const auto TraceDistance{ TraceVector.Size() };

	if (TraceDistance <= UE_KINDA_SMALL_NUMBER)
	{
		TraceDistanceRatio = 1.0f;
		return TraceResult;
	}

	const auto TargetTraceDistanceRatio{ UE_REAL_TO_FLOAT(FVector::Dist(TraceStart, TraceResult) / TraceDistance) };

	TraceDistanceRatio = TargetTraceDistanceRatio <= TraceDistanceRatio
		? TargetTraceDistanceRatio
		: UAlsMath::ExponentialDecay(TraceDistanceRatio, TargetTraceDistanceRatio, DeltaTime,
			Settings->ThirdPerson.TraceDistanceInterpolationSpeed);

	return TraceStart + TraceVector * TraceDistanceRatio;
}

bool UAlsGameplayCameraState::TryAdjustLocationBlockedByGeometry(FVector& Location, const bool bDisplayDebugCameraTraces) const
{
	// Based on ComponentEncroachesBlockingGeometry_WithAdjustment().

	const auto MeshScale{ UE_REAL_TO_FLOAT(Character->GetMesh()->GetComponentScale().Z) };
	const auto CollisionShape{ FCollisionShape::MakeSphere((Settings->ThirdPerson.TraceRadius + 1.0f) * MeshScale) };

	check(Overlaps.IsEmpty())

	ON_SCOPE_EXIT
	{
		Overlaps.Reset();
	};

	static const FName OverlapMultiTraceTag{ FString::Printf(TEXT("%hs (Overlap Multi)"), __FUNCTION__) };

	if (!GetWorld()->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
		CollisionShape, { OverlapMultiTraceTag, false, Character.Get() }))
	{
		return false;
	}

	auto Adjustment{ FVector::ZeroVector };
	auto bAnyValidBlock{ false };

	FMTDResult MtdResult;

	for (const auto& Overlap : Overlaps)
	{
		if (!Overlap.Component.IsValid() || Overlap.Component->GetCollisionResponseToChannel(Settings->ThirdPerson.TraceChannel) != ECR_Block)
		{
			continue;
		}

		const auto* OverlapBody{ Overlap.Component->GetBodyInstance(NAME_None, true, Overlap.ItemIndex) };

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

	auto AdjustmentDirection{ Adjustment };

	if (!AdjustmentDirection.Normalize() ||
		((Character->GetActorLocation() - Location).GetSafeNormal() | AdjustmentDirection) < -UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebugCameraTraces)
	{
		DrawDebugLine(GetWorld(), Location, Location + Adjustment,
			FLinearColor{ 0.0f, 0.75f, 1.0f }.ToFColor(true),
			false, 5.0f, 0, UAlsUtility::DrawLineThickness);
	}
#endif

	Location += Adjustment;

	static const FName FreeSpaceTraceTag{ FString::Printf(TEXT("%hs (Free Space Overlap)"), __FUNCTION__) };

	return !GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, Settings->ThirdPerson.TraceChannel,
		FCollisionShape::MakeSphere(Settings->ThirdPerson.TraceRadius * MeshScale),
		{ FreeSpaceTraceTag, false, Character.Get() });
}

FVector UAlsGameplayCameraState::GetFirstPersonCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(Settings->FirstPerson.CameraSocketName);
}

FVector UAlsGameplayCameraState::GetEyeCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(Settings->FirstPerson.bLeftDominantEye
												   ? Settings->FirstPerson.LeftEyeCameraSocketName
												   : Settings->FirstPerson.RightEyeCameraSocketName);
}

FVector UAlsGameplayCameraState::GetThirdPersonPivotLocation() const
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

	return (FirstPivotLocation + Mesh->GetSocketLocation(Settings->ThirdPerson.SecondPivotSocketName)) * 0.5f;
}

FVector UAlsGameplayCameraState::GetThirdPersonTraceStartLocation() const
{
	return Character->GetMesh()->GetSocketLocation(ShoulderMode == AlsCameraShoulderModeTags::Right
		                                           ? Settings->ThirdPerson.TraceShoulderRightSocketName
		                                           : Settings->ThirdPerson.TraceShoulderLeftSocketName);
}

void UAlsGameplayCameraState::UpdateAimingFirstPersonCamera(float AimingAmount, const FRotator& TargetRotation)
{
	if (AimingAmount > 0.0f && Character->HasSight())
	{
		const auto EyeCameraLocation{ GetEyeCameraLocation() };
		FVector SightLoc;
		FRotator SightRot;
		Character->GetSightLocAndRot(SightLoc, SightRot);
		SightRot.Roll = TargetRotation.Roll;
		SightLoc = FVector::PointPlaneProject(SightLoc, EyeCameraLocation, SightRot.Vector());
		if (AimingAmount >= 1.0f)
		{
			CameraLocation = SightLoc - SightRot.Vector() * Settings->FirstPerson.RetreatDistance;
			CameraRotation = SightRot;
			LastFullAimSightRot = SightRot;
			bFullAim = true;
			return;
		}
		else
		{
			auto EyeAlpha = UAlsMath::Clamp01(AimingAmount / Settings->FirstPerson.ADSThreshold);
			auto SightAlpha = UAlsMath::Clamp01((AimingAmount - Settings->FirstPerson.ADSThreshold) /
				(1.0f - Settings->FirstPerson.ADSThreshold));
			if (bFullAim)
			{
				SightRotOffset = LastFullAimSightRot - TargetRotation;
			}
			auto RotOffset{ FMath::Lerp(FRotator::ZeroRotator, SightRotOffset, EyeAlpha) };
			CameraRotation = FRotator(FQuat::Slerp((TargetRotation + RotOffset).Quaternion(), SightRot.Quaternion(), SightAlpha));
			auto LocOffset = CameraRotation.Vector() * Settings->FirstPerson.RetreatDistance;
			auto EyeLoc = FMath::Lerp(GetFirstPersonCameraLocation(), EyeCameraLocation, EyeAlpha);
			CameraLocation = FMath::Lerp(EyeLoc, SightLoc, SightAlpha) - LocOffset;
			bFullAim = false;
			return;
		}
	}

	auto Offset = TargetRotation.Vector() * Settings->FirstPerson.RetreatDistance;
	CameraLocation = GetFirstPersonCameraLocation() - Offset;
	CameraRotation = TargetRotation;
	SightRotOffset = FRotator::ZeroRotator;
	bFullAim = false;
}

void UAlsGameplayCameraState::UpdateFocalLength()
{
	static const FName MainTraceTag{FString::Printf(TEXT("%hs (Main Trace)"), __FUNCTION__)};
	const auto CollisionShape{FCollisionShape::MakeSphere(Settings->FocusTraceRadius)};

	auto EyeVec = CameraRotation.Vector();
	FVector TraceStart = CameraLocation + EyeVec * Settings->MinFocalLength;
	if (Character->GetViewMode() != AlsViewModeTags::FirstPerson)
	{
		auto ProjectedLocation = FVector::PointPlaneProject(CameraLocation, GetFirstPersonCameraLocation(), -EyeVec);
		auto ProjectedLocationDistance = FVector::Distance(TraceStart, ProjectedLocation);
		TraceStart = ProjectedLocation + EyeVec * FMath::Max(-ProjectedLocationDistance, Settings->ThirdPerson.FocusTraceStartOffset);
	}
	FVector TraceEnd{CameraLocation + EyeVec * Settings->MaxFocalLength};
	FVector TraceResult{TraceEnd};

	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->FocusTraceChannel,
										 CollisionShape, {MainTraceTag, false, Character.Get()}))
	{
		TraceResult = Hit.Location;
		if (Hit.HasValidHitObjectHandle())
		{
			bIsFocusPawn = IsValid(Cast<APawn>(Hit.GetActor()));
		}
	}
	else
	{
		bIsFocusPawn = false;
	}

	FocalLength = FMath::Max(Settings->MinFocalLength, FVector::Distance(TraceResult, CameraLocation));
}

void UAlsGameplayCameraState::UpdateADSCameraShake(float FirstPersonOverride, float AimingAmount)
{
	auto GetCameraManager = [this]() {
		const auto* PlayerController{Character.IsValid() ? Cast<APlayerController>(Character->GetController()) : nullptr};
		return PlayerController && IsValid(PlayerController) ? PlayerController->PlayerCameraManager.Get() : nullptr;
	};

	if (!CurrentADSCameraShake && AimingAmount > Settings->FirstPerson.ADSThreshold && IsValid(Settings->FirstPerson.ADSCameraShakeClass) &&
		Character->HasSight() && (FAnimWeight::IsFullWeight(FirstPersonOverride) || bInAutoFPP))
	{
		auto* CameraManager{GetCameraManager()};
		if (CameraManager)
		{
			CurrentADSCameraShake = CameraManager->StartCameraShake(Settings->FirstPerson.ADSCameraShakeClass, Settings->FirstPerson.ADSCameraShakeScale);
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

void UAlsGameplayCameraState::RefreshTanHalfFov(float DeltaTime)
{
	if (Character.IsValid())
	{
		auto* PlayerController{Cast<APlayerController>(Character->GetController())};
		if (GameplayCameraComponent.IsValid() && IsValid(PlayerController))
		{
			auto CameraSystemEvaluator = GameplayCameraComponent->GetCameraSystemEvaluator();
			if (CameraSystemEvaluator.IsValid())
			{
				const auto& Result = CameraSystemEvaluator->GetEvaluatedResult();
				TanHalfVfov = FMath::Tan(FMath::DegreesToRadians(Result.CameraPose.GetEffectiveFieldOfView()) * 0.5f);
				if (Result.CameraPose.GetPanoramic())
				{
					int32 SizeX, SizeY;
					PlayerController->GetViewportSize(SizeX, SizeY);
					float AspectRatio{SizeX * (1 - Result.CameraPose.GetPanoramaSideViewRate() * 2 / 3) / (float)SizeY};
					TanHalfVfov /= AspectRatio;
				}
			}
		}
	}
}

void UAlsGameplayCameraState::SetDesiredViewMode(const FGameplayTag& NewDesiredViewMode)
{
	DesiredViewMode = NewDesiredViewMode;
}

void UAlsGameplayCameraState::SetConfirmedDesiredViewMode(const FGameplayTag& NewConfirmedDesiredViewMode)
{
	if (ConfirmedDesiredViewMode == NewConfirmedDesiredViewMode || Character->GetLocalRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	ConfirmedDesiredViewMode = NewConfirmedDesiredViewMode;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ConfirmedDesiredViewMode, this)

	if (Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSetConfirmedDesiredViewMode(NewConfirmedDesiredViewMode);
	}
}

void UAlsGameplayCameraState::ServerSetConfirmedDesiredViewMode_Implementation(const FGameplayTag& NewViewMode)
{
	SetConfirmedDesiredViewMode(NewViewMode);
}

void UAlsGameplayCameraState::SetShoulderMode(const FGameplayTag& NewShoulderMode)
{
	if (ShoulderMode == NewShoulderMode || Character->GetLocalRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	ShoulderMode = NewShoulderMode;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ShoulderMode, this)

	if (Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSetShoulderMode(NewShoulderMode);
	}
}

void UAlsGameplayCameraState::ServerSetShoulderMode_Implementation(const FGameplayTag& NewShoulderMode)
{
	SetShoulderMode(NewShoulderMode);
}

void UAlsGameplayCameraState::ToggleShoulder()
{
	if (ShoulderMode != AlsCameraShoulderModeTags::Center)
	{
		SetShoulderMode(ShoulderMode == AlsCameraShoulderModeTags::Right ? AlsCameraShoulderModeTags::Left : AlsCameraShoulderModeTags::Right);
	}
}
