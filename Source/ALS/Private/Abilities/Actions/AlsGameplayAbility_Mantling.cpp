// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Actions/AlsGameplayAbility_Mantling.h"
#include "Abilities/Tasks/AlsAbilityTask_Tick.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsAnimationInstance.h"
#include "RootMotionSources/AlsRootMotionSource_Mantling.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Mantling)

TMap<FGameplayAbilitySpecHandle, FAlsMantlingParameters> UAlsGameplayAbility_Mantling::ParameterMap;

UAlsGameplayAbility_Mantling::UAlsGameplayAbility_Mantling(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	AbilityTags.AddTag(AlsLocomotionActionTags::Mantling);
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::Mantling);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Root);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Mantling);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Rolling);
	ActivationBlockedTags.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
	ActivationBlockedTags.AddTag(AlsLocomotionActionTags::Dying);

	MantlingTraceResponses.WorldStatic = ECR_Block;
	MantlingTraceResponses.WorldDynamic = ECR_Block;
	MantlingTraceResponses.Destructible = ECR_Block;
}

bool UAlsGameplayAbility_Mantling::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
													  const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
													  OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (ParameterMap.Contains(Handle))
	{
		return true;
	}

	FAlsMantlingParameters Params;
	if (CanMantle(Handle, *ActorInfo, Params) && CanMantleByParameter(*ActorInfo, Params))
	{
		// The idea of putting the mutable member variable into the ActivateAbility method does not work well.
		// The memory area is cleared, or if it is a different instance from when CanActivateAbility is called,
		// anyway the contents of the parameter are lost.
		// Therefore, I am passing it with a static variable map.
		CommitParameter(Handle, Params);
		return true;
	}
	return false;
}

bool UAlsGameplayAbility_Mantling::CanMantle(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FAlsMantlingParameters& Params) const
{
	auto* Character{Cast<AAlsCharacter>(ActorInfo.OwnerActor)};
	if (!IsValid(Character))
	{
		return false;
	}

	const auto ActorLocation{Character->GetActorLocation()};
	const auto ActorYawAngle{UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw))};
	const auto& LocomotionState{Character->GetLocomotionState()};

	float ForwardTraceAngle;
	if (LocomotionState.bHasSpeed)
	{
		ForwardTraceAngle = LocomotionState.bHasInput
			                    ? LocomotionState.VelocityYawAngle +
			                      FMath::ClampAngle(LocomotionState.InputYawAngle - LocomotionState.VelocityYawAngle,
			                                        -MaxReachAngle, MaxReachAngle)
			                    : LocomotionState.VelocityYawAngle;
	}
	else
	{
		ForwardTraceAngle = LocomotionState.bHasInput ? LocomotionState.InputYawAngle : ActorYawAngle;
	}

	const auto ForwardTraceDeltaAngle{FRotator3f::NormalizeAxis(ForwardTraceAngle - ActorYawAngle)};
	if (FMath::Abs(ForwardTraceDeltaAngle) > TraceAngleThreshold)
	{
		return false;
	}

	const auto ForwardTraceDirection{
		UAlsMath::AngleToDirectionXY(ActorYawAngle + FMath::ClampAngle(ForwardTraceDeltaAngle, -MaxReachAngle, MaxReachAngle))
	};

#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebug{UAlsUtility::ShouldDisplayDebugForActor(Character, UAlsConstants::MantlingDebugDisplayName())};
#endif

	const auto* Capsule{Character->GetCapsuleComponent()};

	const auto CapsuleScale{Capsule->GetComponentScale().Z};
	const auto CapsuleRadius{Capsule->GetScaledCapsuleRadius()};
	const auto CapsuleHalfHeight{Capsule->GetScaledCapsuleHalfHeight()};

	const FVector CapsuleBottomLocation{ActorLocation.X, ActorLocation.Y, ActorLocation.Z - CapsuleHalfHeight};

	const auto TraceCapsuleRadius{CapsuleRadius - 1.0f};

	bool bInAir{Character->HasMatchingGameplayTag(AlsLocomotionModeTags::InAir)};

	const FAlsMantlingTraceSettings& TraceSettings{bInAir ? InAirTrace : GroundedTrace};

	const auto LedgeHeightDelta{UE_REAL_TO_FLOAT((TraceSettings.LedgeHeight.GetMax() - TraceSettings.LedgeHeight.GetMin()) * CapsuleScale)};

	// Trace forward to find an object the character cannot walk on.

	static const FName ForwardTraceTag{FString::Printf(TEXT("%hs (Forward Trace)"), __FUNCTION__)};

	auto ForwardTraceStart{CapsuleBottomLocation - ForwardTraceDirection * CapsuleRadius};
	ForwardTraceStart.Z += (TraceSettings.LedgeHeight.X + TraceSettings.LedgeHeight.Y) *
		0.5f * CapsuleScale - UCharacterMovementComponent::MAX_FLOOR_DIST;

	auto ForwardTraceEnd{ForwardTraceStart + ForwardTraceDirection * (CapsuleRadius + (TraceSettings.ReachDistance + 1.0f) * CapsuleScale)};

	const auto ForwardTraceCapsuleHalfHeight{LedgeHeightDelta * 0.5f};

	auto* World{Character->GetWorld()};

	FHitResult ForwardTraceHit;
	World->SweepSingleByChannel(ForwardTraceHit, ForwardTraceStart, ForwardTraceEnd,
	                            FQuat::Identity, MantlingTraceChannel,
	                            FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight),
	                            {ForwardTraceTag, false, Character}, MantlingTraceResponses);

	auto* TargetPrimitive{ForwardTraceHit.GetComponent()};

	if (!ForwardTraceHit.IsValidBlockingHit() ||
	    !IsValid(TargetPrimitive) ||
	    TargetPrimitive->GetComponentVelocity().SizeSquared() > FMath::Square(TargetPrimitiveSpeedThreshold) ||
	    !TargetPrimitive->CanCharacterStepUp(Character) ||
		Character->GetCharacterMovement()->IsWalkable(ForwardTraceHit))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(World, ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
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
	World->SweepSingleByChannel(DownwardTraceHit, DownwardTraceStart, DownwardTraceEnd, FQuat::Identity,
	                            MantlingTraceChannel, FCollisionShape::MakeSphere(TraceCapsuleRadius),
	                            {DownwardTraceTag, false, Character}, MantlingTraceResponses);

	const auto SlopeAngleCos{UE_REAL_TO_FLOAT(DownwardTraceHit.ImpactNormal.Z)};

	// The approximate slope angle is used in situations where the normal slope angle cannot convey
	// the true nature of the surface slope, for example, for a 45 degree staircase the slope
	// angle will always be 90 degrees, while the approximate slope angle will be ~45 degrees.

	auto ApproximateSlopeNormal{DownwardTraceHit.Location - DownwardTraceHit.ImpactPoint};
	ApproximateSlopeNormal.Normalize();

	const auto ApproximateSlopeAngleCos{UE_REAL_TO_FLOAT(ApproximateSlopeNormal.Z)};

	if (SlopeAngleCos < SlopeAngleThresholdCos ||
	    ApproximateSlopeAngleCos < SlopeAngleThresholdCos ||
	    !Character->GetCharacterMovement()->IsWalkable(DownwardTraceHit))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(World, ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit, {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);

			UAlsUtility::DrawDebugSweepSingleSphere(World, DownwardTraceStart, DownwardTraceEnd, TraceCapsuleRadius,
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

	if (World->OverlapBlockingTestByChannel(TargetCapsuleLocation, FQuat::Identity, MantlingTraceChannel,
	                                             FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
	                                             {TargetLocationTraceTag, false, Character}, MantlingTraceResponses))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(World, ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit, {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);

			UAlsUtility::DrawDebugSweepSingleSphere(World, DownwardTraceStart, DownwardTraceEnd, TraceCapsuleRadius,
			                                        false, DownwardTraceHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                        TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);

			DrawDebugCapsule(World, TargetCapsuleLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity,
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

	if (World->OverlapBlockingTestByChannel(StartLocation, FQuat::Identity, MantlingTraceChannel,
	                                        FCollisionShape::MakeCapsule(TraceCapsuleRadius, StartLocationTraceCapsuleHalfHeight),
	                                        {StartLocationTraceTag, false, Character}, MantlingTraceResponses))
	{
#if ENABLE_DRAW_DEBUG
		if (bDisplayDebug)
		{
			UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(World, ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
			                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit,
			                                                    {0.0f, 0.25f, 1.0f},
			                                                    {0.0f, 0.75f, 1.0f}, TraceSettings.bDrawFailedTraces ? 5.0f : 0.0f);

			UAlsUtility::DrawDebugSweepSingleSphere(World, DownwardTraceStart, DownwardTraceEnd, TraceCapsuleRadius,
			                                        false, DownwardTraceHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                        TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);

			DrawDebugCapsule(World, StartLocation, StartLocationTraceCapsuleHalfHeight, TraceCapsuleRadius, FQuat::Identity,
			                 FLinearColor{1.0f, 0.5f, 0.0f}.ToFColor(true), false, TraceSettings.bDrawFailedTraces ? 10.0f : 0.0f);
		}
#endif

		return false;
	}

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebug)
	{
		UAlsUtility::DrawDebugSweepSingleCapsuleAlternative(World, ForwardTraceStart, ForwardTraceEnd, TraceCapsuleRadius,
		                                                    ForwardTraceCapsuleHalfHeight, true, ForwardTraceHit,
		                                                    {0.0f, 0.25f, 1.0f}, {0.0f, 0.75f, 1.0f}, 5.0f);

		UAlsUtility::DrawDebugSweepSingleSphere(World, DownwardTraceStart, DownwardTraceEnd,
		                                        TraceCapsuleRadius, true, DownwardTraceHit,
		                                        {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f}, 7.5f);
	}
#endif

	const auto TargetRotation{TargetDirection.ToOrientationQuat()};
	const auto MantlingHeight{UE_REAL_TO_FLOAT((TargetLocation.Z - CapsuleBottomLocation.Z) / CapsuleScale)};

	Params.TargetPrimitive = TargetPrimitive;
	Params.MantlingHeight = MantlingHeight;

	// Determine the mantling type by checking the movement mode and mantling height.

	Params.MantlingType = bInAir ? EAlsMantlingType::InAir
								 : MantlingHeight > MantlingHighHeightThreshold
									? EAlsMantlingType::High
									: EAlsMantlingType::Low;

	// If the target primitive can't move, then use world coordinates to save
	// some performance by skipping some coordinate space transformations later.

	if (MovementBaseUtility::UseRelativeLocation(TargetPrimitive))
	{
		const auto TargetRelativeTransform{
			TargetPrimitive->GetComponentTransform().GetRelativeTransform({TargetRotation, TargetLocation})
		};

		Params.TargetRelativeLocation = TargetRelativeTransform.GetLocation();
		Params.TargetRelativeRotation = TargetRelativeTransform.Rotator();
	}
	else
	{
		Params.TargetRelativeLocation = TargetLocation;
		Params.TargetRelativeRotation = TargetRotation.Rotator();
	}

	return true;
}

void UAlsGameplayAbility_Mantling::CommitParameter(const FGameplayAbilitySpecHandle Handle, const FAlsMantlingParameters& Parameters) const
{
	ParameterMap.Add(Handle, Parameters);
}

void UAlsGameplayAbility_Mantling::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !ParameterMap.Contains(Handle))
	{
		return;
	}

	const auto& Parameters = ParameterMap[Handle];

	ON_SCOPE_EXIT
	{
		ParameterMap.Remove(Handle);
	};

	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* AnimInstance{Character->GetAlsAnimationInstace()};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};
	
	const auto* MantlingSettings{SelectMantlingSettings(Parameters)};

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

	const auto ActorFeetLocationOffset{CharacterMovement->GetActorFeetLocation() - TargetTransform.GetLocation()};
	const auto ActorRotationOffset{TargetTransform.GetRotation().Inverse() * Character->GetActorQuat()};

	// Reset network smoothing.

	CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;

	Character->GetMesh()->SetRelativeLocationAndRotation(Character->GetBaseTranslationOffset(),
														 Character->GetMesh()->IsUsingAbsoluteRotation()
															? Character->GetActorQuat() * Character->GetBaseRotationOffset()
															: Character->GetBaseRotationOffset(), false, nullptr, ETeleportType::TeleportPhysics);

	// Clear the character movement mode and set the locomotion action to mantling.

	CharacterMovement->SetMovementMode(MOVE_Custom);
	CharacterMovement->SetMovementModeLocked(true);

	CharacterMovement->SetBase(Parameters.TargetPrimitive.Get());

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

	RootMotionSourceId = CharacterMovement->ApplyRootMotionSource(RootMotionSource);

	// Play the animation montage if valid.
	PlayMontage(ActivationInfo, MantlingSettings->Montage, PlayRate, NAME_None, StartTime, Handle, ActorInfo);

	if (CurrentMotangeDuration <= 0.0f)
	{
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsActive())
	{
		TickTask = UAlsAbilityTask_Tick::New(this, FName(TEXT("UAlsGameplayAbility_Mantling")));
		if (TickTask.IsValid())
		{
			TickTask->OnTick.AddDynamic(this, &ThisClass::Tick);
			TickTask->ReadyForActivation();
		}
	}
}

void UAlsGameplayAbility_Mantling::Tick_Implementation(const float DeltaTime)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};

	if (CharacterMovement->MovementMode != MOVE_Custom)
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
			ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, true);
		return;
	}

	const auto* RootMotionSource{
		StaticCastSharedPtr<FAlsRootMotionSource_Mantling>(CharacterMovement->GetRootMotionSourceByID(RootMotionSourceId)).Get()
	};

	if (RootMotionSource != nullptr && !RootMotionSource->TargetPrimitive.IsValid())
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
			ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, true);

		if (bStartRagdollingOnTargetPrimitiveDestruction)
		{
			GetAlsAbilitySystemComponentFromActorInfo()->TryActivateAbilitiesBySingleTag(TryActiveOnPrimitiveDestruction);
		}
	}
}

void UAlsGameplayAbility_Mantling::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											  const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* AnimInstance{Character->GetAlsAnimationInstace()};
	auto CharacterMovement{Character->GetAlsCharacterMovement()};
	
	auto* RootMotionSource{
		StaticCastSharedPtr<FAlsRootMotionSource_Mantling>(CharacterMovement->GetRootMotionSourceByID(RootMotionSourceId)).Get()
	};

	if (RootMotionSource != nullptr)
	{
		RootMotionSource->Status.SetFlag(ERootMotionSourceStatusFlags::MarkedForRemoval);
	}

	RootMotionSourceId = 0;

	CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

	CharacterMovement->SetMovementModeLocked(false);
	CharacterMovement->SetMovementMode(MOVE_Walking);

	//ForceNetUpdate();
}

float UAlsGameplayAbility_Mantling::CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, const float MantlingHeight) const
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

bool UAlsGameplayAbility_Mantling::CanMantleByParameter_Implementation(const FGameplayAbilityActorInfo& ActorInfo, const FAlsMantlingParameters& Parameter) const
{
	return true;
}

UAlsMantlingSettings* UAlsGameplayAbility_Mantling::SelectMantlingSettings_Implementation(const FAlsMantlingParameters& Parameters) const
{
	return nullptr;
}

#if WITH_EDITOR
void UAlsGameplayAbility_Mantling::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, SlopeAngleThreshold))
	{
		SlopeAngleThresholdCos = FMath::Cos(FMath::DegreesToRadians(SlopeAngleThreshold));
	}
	else if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(ThisClass, MantlingTraceResponseChannels))
	{
		return;
	}

	MantlingTraceResponses.SetAllChannels(ECR_Ignore);

	for (const auto CollisionChannel : MantlingTraceResponseChannels)
	{
		MantlingTraceResponses.SetResponse(CollisionChannel, ECR_Block);
	}
}
#endif
