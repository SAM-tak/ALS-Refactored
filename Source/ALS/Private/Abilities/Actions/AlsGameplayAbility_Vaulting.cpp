// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Actions/AlsGameplayAbility_Vaulting.h"
#include "Abilities/Tasks/AlsAbilityTask_Tick.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsAnimationInstance.h"
#include "AlsMotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Vaulting)

TMap<FGameplayAbilitySpecHandle, FAlsVaultingParameters> UAlsGameplayAbility_Vaulting::ParameterMap;

UAlsGameplayAbility_Vaulting::UAlsGameplayAbility_Vaulting(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(AlsLocomotionActionTags::Vaulting));
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::Vaulting);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Root);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Vaulting);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Rolling);
	ActivationBlockedTags.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
	ActivationBlockedTags.AddTag(AlsLocomotionActionTags::Dying);

	VaultingTraceResponses.WorldStatic = ECR_Block;
	VaultingTraceResponses.WorldDynamic = ECR_Block;
	VaultingTraceResponses.Destructible = ECR_Block;
}

bool UAlsGameplayAbility_Vaulting::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	FAlsVaultingParameters Params;
	if (CanVault(Handle, *ActorInfo, Params) && CanVaultByParameter(*ActorInfo, Params))
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

bool UAlsGameplayAbility_Vaulting::CanVault(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FAlsVaultingParameters& Params) const
{
	auto* Character{Cast<AAlsCharacter>(ActorInfo.OwnerActor)};
	if (!IsValid(Character))
	{
		return false;
	}

	bool bInAir{Character->HasMatchingGameplayTag(AlsLocomotionModeTags::InAir)};

	if (bInAir)
	{
		return false;
	}

	const auto ActorLocation{Character->GetActorLocation()};
	const auto ActorYawAngle{UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw))};
	const auto& LocomotionState{Character->GetLocomotionState()};
	
	if (!LocomotionState.bHasInput)
	{
		return false;
	}
	
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
	bool bDisplayDebug{UAlsUtility::ShouldDisplayDebugForActor(Character, UAlsConstants::MantlingDebugDisplayName())};
#endif

	const auto* Capsule{Character->GetCapsuleComponent()};

	const auto CapsuleScale{Capsule->GetComponentScale().Z};
	const auto CapsuleRadius{Capsule->GetScaledCapsuleRadius()};
	const auto CapsuleHalfHeight{Capsule->GetScaledCapsuleHalfHeight()};

	const FVector CapsuleBottomLocation{ActorLocation.X, ActorLocation.Y, ActorLocation.Z - CapsuleHalfHeight};

	const auto TraceCapsuleRadius{CapsuleRadius - 1.0f};

	const auto LedgeHeightDelta{UE_REAL_TO_FLOAT((TraceSettings.LedgeHeight.GetMax() - TraceSettings.LedgeHeight.GetMin()) * CapsuleScale)};

	// Trace forward to find an object the character cannot walk on.

	static const FName ForwardTraceTag{FString::Printf(TEXT("%hs (Forward Trace)"), __FUNCTION__)};

	auto ForwardTraceStart{CapsuleBottomLocation - ForwardTraceDirection * CapsuleRadius};
	ForwardTraceStart.Z += (TraceSettings.LedgeHeight.X + TraceSettings.LedgeHeight.Y) * 0.5f * CapsuleScale - UCharacterMovementComponent::MAX_FLOOR_DIST;

	auto ForwardTraceEnd{ForwardTraceStart + ForwardTraceDirection * (CapsuleRadius + (TraceSettings.ReachDistance + 1.0f) * CapsuleScale)};

	const auto ForwardTraceCapsuleHalfHeight{LedgeHeightDelta * 0.5f};

	auto* World{Character->GetWorld()};

	FHitResult ForwardTraceHit;
	World->SweepSingleByChannel(ForwardTraceHit, ForwardTraceStart, ForwardTraceEnd,
	                            FQuat::Identity, VaultingTraceChannel,
	                            FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight),
	                            {ForwardTraceTag, false, Character}, VaultingTraceResponses);

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
	                            VaultingTraceChannel, FCollisionShape::MakeSphere(TraceCapsuleRadius),
	                            {DownwardTraceTag, false, Character}, VaultingTraceResponses);

	// Check that there is enough free space for the capsule at the target location.

	static const FName TargetLocationTraceTag{FString::Printf(TEXT("%hs (Target Location Overlap)"), __FUNCTION__)};

	const FVector TargetLocation{
		DownwardTraceHit.Location.X,
		DownwardTraceHit.Location.Y,
		DownwardTraceHit.ImpactPoint.Z + UCharacterMovementComponent::MIN_FLOOR_DIST
	};

	const FVector TargetCapsuleLocation{TargetLocation.X, TargetLocation.Y, TargetLocation.Z + CapsuleHalfHeight};

	if (World->OverlapBlockingTestByChannel(TargetCapsuleLocation, FQuat::Identity, VaultingTraceChannel,
	                                        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
	                                        {TargetLocationTraceTag, false, Character}, VaultingTraceResponses))
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

	if (World->OverlapBlockingTestByChannel(StartLocation, FQuat::Identity, VaultingTraceChannel,
	                                        FCollisionShape::MakeCapsule(TraceCapsuleRadius, StartLocationTraceCapsuleHalfHeight),
	                                        {StartLocationTraceTag, false, Character}, VaultingTraceResponses))
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

	static const FName MidSpaceTraceTag{FString::Printf(TEXT("%hs (Mid Space Trace)"), __FUNCTION__) };

	const FVector MidSpaceTraceStart{TargetLocation + FVector{0.f, 0.f, TraceCapsuleRadius + UCharacterMovementComponent::MIN_FLOOR_DIST}};
	const FVector MidSpaceTraceEnd{MidSpaceTraceStart + TargetDirection * ReleaseDistance};

	FHitResult MidSpaceTestHit;
	World->SweepSingleByChannel(MidSpaceTestHit, MidSpaceTraceStart, MidSpaceTraceEnd, FQuat::Identity,
								VaultingTraceChannel, FCollisionShape::MakeSphere(TraceCapsuleRadius),
								{MidSpaceTraceTag, false, Character}, VaultingTraceResponses);

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebug)
	{
		UAlsUtility::DrawDebugSweepSingleSphere(World, MidSpaceTraceStart, MidSpaceTraceEnd, TraceCapsuleRadius,
			                                    MidSpaceTestHit.IsValidBlockingHit(), MidSpaceTestHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                    MidSpaceTestHit.IsValidBlockingHit() || TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);
	}
#endif

	auto Distance{ReleaseDistance};

	if (MidSpaceTestHit.IsValidBlockingHit())
	{
		if (MidSpaceTestHit.Distance < MinimumSpace)
		{
			return false;
		}
		Distance = MidSpaceTestHit.Distance;
	}

	static const FName EndLocationTraceTag{FString::Printf(TEXT("%hs (End Location Overlap)"), __FUNCTION__)};

	const FVector EndLocationTraceStart{TargetLocation + TargetDirection * Distance + FVector{0.f, 0.f, TraceCapsuleRadius + UCharacterMovementComponent::MIN_FLOOR_DIST}};

	const FVector EndLocationTraceEnd{
		EndLocationTraceStart.X,
		EndLocationTraceStart.Y,
		Character->GetCharacterMovement()->GetActorFeetLocation().Z
	};

	FHitResult EndLocationTraceHit;
	World->SweepSingleByChannel(EndLocationTraceHit, EndLocationTraceStart, EndLocationTraceEnd, FQuat::Identity,
								VaultingTraceChannel, FCollisionShape::MakeSphere(TraceCapsuleRadius),
								{EndLocationTraceTag, false, Character}, VaultingTraceResponses);

#if ENABLE_DRAW_DEBUG
	if (bDisplayDebug)
	{
		UAlsUtility::DrawDebugSweepSingleSphere(World, EndLocationTraceStart, EndLocationTraceEnd, TraceCapsuleRadius,
			                                    EndLocationTraceHit.IsValidBlockingHit(), EndLocationTraceHit, {0.25f, 0.0f, 1.0f}, {0.75f, 0.0f, 1.0f},
			                                    EndLocationTraceHit.IsValidBlockingHit() || TraceSettings.bDrawFailedTraces ? 7.5f : 0.0f);
	}
#endif

	if (EndLocationTraceHit.IsValidBlockingHit() && EndLocationTraceHit.Distance < EndLocationMinimumDepth)
	{
		return false;
	}

	const auto TargetRotation{TargetDirection.ToOrientationQuat()};

	Params.TargetPrimitive = TargetPrimitive;

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

	if (EndLocationTraceHit.IsValidBlockingHit())
	{
		Params.EndTargetLocation = EndLocationTraceHit.ImpactPoint;
	}
	else
	{
		Params.EndTargetLocation = EndLocationTraceEnd;
	}

	return true;
}

void UAlsGameplayAbility_Vaulting::CommitParameter(const FGameplayAbilitySpecHandle Handle, const FAlsVaultingParameters& Parameters) const
{
	ParameterMap.Add(Handle, Parameters);
}

void UAlsGameplayAbility_Vaulting::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!IsActive())
	{
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !ParameterMap.Contains(Handle))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const auto& Parameters = ParameterMap[Handle];

	ON_SCOPE_EXIT
	{
		ParameterMap.Remove(Handle);
	};

	auto* Character{GetAlsCharacterFromActorInfo()};

	auto* MotionWarping{Character->GetMotionWarping()};

	if (!ALS_ENSURE(IsValid(MotionWarping)))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const auto* Capsule{Character->GetCapsuleComponent()};
	const auto CapsuleHalfHeight{Capsule->GetScaledCapsuleHalfHeight()};

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

	MotionWarping->AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName{"Uprise"}, TargetTransform.GetLocation(), TargetTransform.GetRotation().Rotator());

	MotionWarping->AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName{"Release"}, Parameters.EndTargetLocation, TargetTransform.GetRotation().Rotator());

	auto PlayMontageParameter{SelectMontage(Parameters)};

	if (!PlayMontage(PlayMontageParameter))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Reset network smoothing.

	Character->GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;

	Character->GetMesh()->SetRelativeLocationAndRotation(Character->GetBaseTranslationOffset(),
														 Character->GetMesh()->IsUsingAbsoluteRotation()
															? Character->GetActorQuat() * Character->GetBaseRotationOffset()
															: Character->GetBaseRotationOffset(), false, nullptr, ETeleportType::TeleportPhysics);

	// Clear the character movement mode and set the locomotion action to mantling.

	Character->GetCharacterMovement()->FlushServerMoves();
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	Character->GetAlsCharacterMovement()->SetMovementModeLocked(true);

	Character->GetCharacterMovement()->SetBase(Parameters.TargetPrimitive.Get());

	Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	PreviousLocation = Character->GetActorLocation();
	LastVelocity = FVector::ZeroVector;

	TickTask = UAlsAbilityTask_Tick::New(this, FName(TEXT("UAlsGameplayAbility_Vaulting")));
	if (TickTask.IsValid())
	{
		TickTask->OnTick.AddDynamic(this, &ThisClass::Tick);
		TickTask->ReadyForActivation();
	}
}

void UAlsGameplayAbility_Vaulting::Tick_Implementation(const float DeltaTime)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};

	auto CurrentLocation{Character->GetActorLocation()};

	auto Velocity = (CurrentLocation - PreviousLocation) / DeltaTime;

	LastVelocity = FMath::VInterpTo(LastVelocity, Velocity, DeltaTime, 10.0f);

	PreviousLocation = CurrentLocation;

	if (CharacterMovement->MovementMode != MOVE_Custom)
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
		return;
	}
}

void UAlsGameplayAbility_Vaulting::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											  const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* AnimInstance{Character->GetAlsAnimationInstace()};
	auto CharacterMovement{Character->GetAlsCharacterMovement()};
	auto* AbilitySystem{GetAlsAbilitySystemComponentFromActorInfo()};
	const auto* Capsule{Character->GetCapsuleComponent()};

	CharacterMovement->Velocity = LastVelocity;

	CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

	CharacterMovement->SetMovementModeLocked(false);

	FHitResult Hit;
	auto FeetLocation{Character->GetCharacterMovement()->GetActorFeetLocation()};
	Character->GetWorld()->LineTraceSingleByChannel(Hit, Character->GetActorLocation(),
													FeetLocation - FVector{0.f, 0.f, UCharacterMovementComponent::MIN_FLOOR_DIST},
													Capsule->GetCollisionObjectType(),
													{__FUNCTION__, false, Character},
													Capsule->GetCollisionResponseToChannel(Capsule->GetCollisionObjectType()));

	CharacterMovement->SetMovementMode(CharacterMovement->IsWalkable(Hit) ? MOVE_Walking : MOVE_Falling);

	Character->ForceNetUpdate();
}

bool UAlsGameplayAbility_Vaulting::CanVaultByParameter_Implementation(const FGameplayAbilityActorInfo& ActorInfo, const FAlsVaultingParameters& Parameter) const
{
	return true;
}

#if WITH_EDITOR
void UAlsGameplayAbility_Vaulting::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(ThisClass, VaultingTraceResponseChannels))
	{
		return;
	}

	VaultingTraceResponses.SetAllChannels(ECR_Ignore);

	for (const auto& CollisionChannel : VaultingTraceResponseChannels)
	{
		VaultingTraceResponses.SetResponse(CollisionChannel, ECR_Block);
	}
}
#endif
