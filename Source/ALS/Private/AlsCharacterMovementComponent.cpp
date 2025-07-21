#include "AlsCharacterMovementComponent.h"

#include "AlsCharacter.h"
#include "AlsPhysicalAnimationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterMovementComponent)

void FAlsCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& Move, const ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const auto& SavedMove{static_cast<const FAlsSavedMove&>(Move)};

	RotationMode = SavedMove.RotationMode;
	Stance = SavedMove.Stance;
	MaxAllowedGait = SavedMove.MaxAllowedGait;
}

bool FAlsCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& Movement, FArchive& Archive,
											 UPackageMap* Map, const ENetworkMoveType MoveType)
{
	Super::Serialize(Movement, Archive, Map, MoveType);

	NetSerializeOptionalValue(Archive.IsSaving(), Archive, RotationMode, AlsRotationModeTags::ViewDirection.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Stance, AlsStanceTags::Standing.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, MaxAllowedGait, AlsGaitTags::Walking.GetTag(), Map);

	return !Archive.IsError();
}

FAlsCharacterNetworkMoveDataContainer::FAlsCharacterNetworkMoveDataContainer()
{
	NewMoveData = &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData = &MoveData[2];
}

void FAlsSavedMove::Clear()
{
	Super::Clear();

	RotationMode = AlsRotationModeTags::ViewDirection;
	Stance = AlsStanceTags::Standing;
	MaxAllowedGait = AlsGaitTags::Walking;
}

void FAlsSavedMove::SetMoveFor(ACharacter* Character, const float NewDeltaTime, const FVector& NewAcceleration,
							   FNetworkPredictionData_Client_Character& PredictionData)
{
	Super::SetMoveFor(Character, NewDeltaTime, NewAcceleration, PredictionData);

	const auto* Movement{Cast<UAlsCharacterMovementComponent>(Character->GetCharacterMovement())};
	if (IsValid(Movement))
	{
		RotationMode = Movement->RotationMode;
		Stance = Movement->Stance;
		MaxAllowedGait = Movement->MaxAllowedGait;
		bWantsToLie = Movement->bWantsToLie;
	}
}

bool FAlsSavedMove::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, const float MaxDelta) const
{
	const auto* NewMove{static_cast<FAlsSavedMove*>(NewMovePtr.Get())};

	return RotationMode == NewMove->RotationMode &&
		   Stance == NewMove->Stance &&
		   MaxAllowedGait == NewMove->MaxAllowedGait &&
		   Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

void FAlsSavedMove::CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character,
								APlayerController* Player, const FVector& PreviousStartLocation)
{
	// Calling Super::CombineWith() will force change the character's rotation to the rotation from the previous move, which is
	// undesirable because it will erase our rotation changes made in the AAlsCharacter class. So, to keep the rotation unchanged,
	// we simply override the saved rotations with the current rotation, and after calling Super::CombineWith() we restore them.

	const auto OriginalRotation{PreviousMove->StartRotation};
	const auto OriginalRelativeRotation{PreviousMove->StartAttachRelativeRotation};

	const auto* UpdatedComponent{Character->GetCharacterMovement()->UpdatedComponent.Get()};

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation = UpdatedComponent->GetComponentRotation();
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = UpdatedComponent->GetRelativeRotation();

	Super::CombineWith(PreviousMove, Character, Player, PreviousStartLocation);

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation = OriginalRotation;
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = OriginalRelativeRotation;
}

void FAlsSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	auto* Movement{Cast<UAlsCharacterMovementComponent>(Character->GetCharacterMovement())};
	if (IsValid(Movement))
	{
		Movement->RotationMode = RotationMode;
		Movement->Stance = Stance;
		Movement->MaxAllowedGait = MaxAllowedGait;
		Movement->bWantsToLie = bWantsToLie;

		Movement->RefreshGaitSettings();
	}
}

FAlsNetworkPredictionData::FAlsNetworkPredictionData(const UCharacterMovementComponent& Movement) : Super{Movement} {}

FSavedMovePtr FAlsNetworkPredictionData::AllocateNewMove()
{
	return MakeShared<FAlsSavedMove>();
}

UAlsCharacterMovementComponent::UAlsCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetNetworkMoveDataContainer(MoveDataContainer);

	bTickBeforeOwner = true;

	// NetworkMaxSmoothUpdateDistance = 92.0f;
	// NetworkNoSmoothUpdateDistance = 140.0f;

	MaxAcceleration = 1500.0f;
	BrakingFrictionFactor = 0.0f;
	SetCrouchedHalfHeight(56.0f);

	bRunPhysicsWithNoController = true;

	GroundFriction = 4.0f;
	MaxWalkSpeed = 375.0f;
	MaxWalkSpeedCrouched = 200.0f;
	MinAnalogWalkSpeed = 25.0f;
	bCanWalkOffLedgesWhenCrouching = true;

	// bImpartBaseVelocityX = false;
	// bImpartBaseVelocityY = false;
	// bImpartBaseVelocityZ = false;
	// bImpartBaseAngularVelocity = false;

	bIgnoreBaseRotation = true;

	PerchRadiusThreshold = 20.0f;
	PerchAdditionalHeight = 0.0f;
	LedgeCheckThreshold = 0.0f;

	AirControl = 0.15f;

	// https://unrealengine.hatenablog.com/entry/2019/01/16/231404

	FallingLateralFriction = 1.0f;
	JumpOffJumpZFactor = 0.0f;

	bNetworkAlwaysReplicateTransformUpdateTimestamp = true; // Required for view network smoothing.

	RotationRate = FRotator::ZeroRotator;
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;

	bAllowPhysicsRotationDuringAnimRootMotion = true; // Used to allow character rotation while rolling.

	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanFly = true;
	NavMovementProperties.bUseAccelerationForPaths = true;

	bWantsToLie = false;
}

#if WITH_EDITOR
bool UAlsCharacterMovementComponent::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		   Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, RotationRate) &&
		   Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerDesiredRotation) &&
		   Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bOrientRotationToMovement);
}
#endif

void UAlsCharacterMovementComponent::BeginPlay()
{
	ALS_ENSURE_MESSAGE(!bUseControllerDesiredRotation && !bOrientRotationToMovement,
					   TEXT("These settings are not allowed and must be turned off!"));

	Super::BeginPlay();
}

void UAlsCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	auto* Controller{CharacterOwner->GetController()};
	if (IsValid(Controller) && IsValid(MovementSettings) && !PreviousControlRotation.ContainsNaN() && MovementSettings->MaxRotationSpeed > 0)
	{
		// Limit rotation speed
		auto MaxDelta{MovementSettings->MaxRotationSpeed * DeltaTime};
		auto CurrentControlRotation{Controller->GetControlRotation()};
		auto ClampedPitch{FMath::Clamp(FMath::FindDeltaAngleDegrees(PreviousControlRotation.Pitch, CurrentControlRotation.Pitch), -MaxDelta, MaxDelta)};
		auto ClampedYaw{FMath::Clamp(FMath::FindDeltaAngleDegrees(PreviousControlRotation.Yaw, CurrentControlRotation.Yaw), -MaxDelta, MaxDelta)};
		Controller->SetControlRotation(FRotator{
			(PreviousControlRotation.Pitch + ClampedPitch),
			(PreviousControlRotation.Yaw + ClampedYaw),
			CurrentControlRotation.Roll}.Clamp());
	}

	//auto AlsCharacter{GetAlsCharacter()};
	//if (AlsCharacter->GetPhysicalAnimation()->IsRagdolling() && (MovementMode == EMovementMode::MOVE_Custom || MovementMode == EMovementMode::MOVE_None))
	//{
	//	// Proxies get replicated crouch state.
	//	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	//	{
	//		// Check for a change in crouch state. Players toggle crouch by changing bWantsToCrouch.
	//		if (CharacterOwner->bIsCrouched && !bWantsToCrouch)
	//		{
	//			CharacterOwner->bIsCrouched = false;
	//		}
	//		else if (!CharacterOwner->bIsCrouched && bWantsToCrouch)
	//		{
	//			CharacterOwner->bIsCrouched = true;
	//		}
	//	}
	//}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAlsCharacterMovementComponent::SetMovementMode(const EMovementMode NewMovementMode, const uint8 NewCustomMode)
{
	if (!bMovementModeLocked)
	{
		Super::SetMovementMode(NewMovementMode, NewCustomMode);
	}
}

void UAlsCharacterMovementComponent::OnMovementModeChanged(const EMovementMode PreviousMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// This removes some very noticeable changes in the mesh location when the
	// character automatically uncrouches at the end of the roll in the air.

	bCrouchMaintainsBaseLocation = true;
}

bool UAlsCharacterMovementComponent::ShouldPerformAirControlForPathFollowing() const
{
	return !bInputBlocked && Super::ShouldPerformAirControlForPathFollowing();
}

void UAlsCharacterMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	// Ignore the parent implementation of this function and provide our own, because the parent
	// implementation has no effect when we ignore rotation changes in AAlsCharacter::FaceRotation().

	const auto& BasedMovement{CharacterOwner->GetBasedMovement()};

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
												  MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const auto DeltaRotation{(MovementBaseRotation * OldBaseQuat.Inverse()).Rotator()};
		auto NewControlRotation{CharacterOwner->Controller->GetControlRotation()};

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->Controller->SetControlRotation(NewControlRotation);
	}
}

bool UAlsCharacterMovementComponent::ApplyRequestedMove(const float DeltaTime, const float CurrentMaxAcceleration,
														const float MaxSpeed, const float Friction, const float BrakingDeceleration,
														FVector& RequestedAcceleration, float& RequestedSpeed)
{
	return !bInputBlocked && Super::ApplyRequestedMove(DeltaTime, CurrentMaxAcceleration, MaxSpeed, Friction,
													   BrakingDeceleration, RequestedAcceleration, RequestedSpeed);
}

void UAlsCharacterMovementComponent::CalcVelocity(const float DeltaTime, const float Friction,
												  const bool bFluid, const float BrakingDeceleration)
{
	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && UAlsUtility::TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// Offset the velocity to keep it relative to the movement base.
		Velocity = (BaseRotationSpeed * DeltaTime).RotateVector(Velocity);
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

float UAlsCharacterMovementComponent::GetMaxAcceleration() const
{
	// Get the acceleration using the movement curve. This allows for fine control over movement behavior at each speed.

	return IsMovingOnGround() && ALS_ENSURE(IsValid(GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve))
			   ? GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[0].Eval(CalculateGaitAmount())
			   : Super::GetMaxAcceleration();
}

float UAlsCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// Get the deceleration using the movement curve. This allows for fine control over movement behavior at each speed.

	return IsMovingOnGround() && ALS_ENSURE(IsValid(GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve))
			   ? GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[1].Eval(CalculateGaitAmount())
			   : Super::GetMaxBrakingDeceleration();
}

void UAlsCharacterMovementComponent::ControlledCharacterMove(const FVector& InputVector, const float DeltaTime)
{
	Super::ControlledCharacterMove(InputVector, DeltaTime);

	const auto* Controller{CharacterOwner->GetController()};
	if (IsValid(Controller))
	{
		PreviousControlRotation = Controller->GetControlRotation();
	}
}

void UAlsCharacterMovementComponent::PhysicsRotation(const float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);

	if (HasValidData() && (bRunPhysicsWithNoController || IsValid(CharacterOwner->Controller)))
	{
		OnPhysicsRotation.Broadcast(DeltaTime);
	}
}

void UAlsCharacterMovementComponent::PhysWalking(const float DeltaTime, int32 Iterations)
{
	if (ALS_ENSURE(IsValid(GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve)))
	{
		// Get the ground friction using the movement curve. This allows for fine control over movement behavior at each speed.

		GroundFriction = GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[2].Eval(CalculateGaitAmount());
	}

	// TODO Copied with modifications from UCharacterMovementComponent::PhysWalking(). After the
	// TODO release of a new engine version, this code should be updated to match the source code.

	// ReSharper disable All

	// SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);

	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float RemainingTime = DeltaTime;

	// Perform the move
	while ( (RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float TimeTick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= TimeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration = FVector::VectorPlaneProject(Acceleration, -GetGravityDirection());

		static const auto* EnsureAlwaysEnabledConsoleVariable{
			IConsoleManager::Get().FindConsoleVariable(TEXT("p.LedgeMovement.ApplyDirectMove"))
		};
		check(EnsureAlwaysEnabledConsoleVariable != nullptr)

		// Apply acceleration
		const bool bSkipForLedgeMove = bTriedLedgeMove && EnsureAlwaysEnabledConsoleVariable->GetBool();
		if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bSkipForLedgeMove )
		{
			CalcVelocity(TimeTick, GroundFriction, false, GetMaxBrakingDeceleration());
			// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}

		ApplyRootMotionToVelocity(TimeTick);
		// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if( IsFalling() )
		{
			// Root motion could have put us into Falling.
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(RemainingTime+TimeTick, Iterations-1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = TimeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if ( bZeroDelta )
		{
			RemainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, TimeTick, &StepDownResult);

			if ( IsFalling() )
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = UE_REAL_TO_FLOAT(ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - OldLocation).Size());
					RemainingTime += TimeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(RemainingTime,Iterations);
				return;
			}
			else if ( IsSwimming() ) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, TimeTick, RemainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta/TimeTick;
				RemainingTime += TimeTick;
				Iterations--;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, RemainingTime, TimeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				RemainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, TimeTick);
					if (IsMovingOnGround())
					{
						// TODO Start of custom ALS code block.

						ApplyPendingPenetrationAdjustment();

						// TODO End of custom ALS code block.

						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, RemainingTime, TimeTick, Delta, OldLocation);
					}
					return;
				}

				// TODO Start of custom ALS code block.

				ApplyPendingPenetrationAdjustment();

				// TODO End of custom ALS code block.

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && RemainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + MAX_FLOOR_DIST * -GetGravityDirection();
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, TimeTick, RemainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, RemainingTime, TimeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && TimeTick >= MIN_TICK_TIME)
			{
				// TODO Start of custom ALS code block.

				PrePenetrationAdjustmentVelocity = MoveVelocity;
				bPrePenetrationAdjustmentVelocityValid = true;

				// TODO End of custom ALS code block.

				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / TimeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			RemainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}

	// ReSharper restore All
}

void UAlsCharacterMovementComponent::PhysNavWalking(const float DeltaTime, const int32 Iterations)
{
	if (ALS_ENSURE(IsValid(GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve)))
	{
		// Get the ground friction using the movement curve. This allows for fine control over movement behavior at each speed.

		GroundFriction = GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[2].Eval(CalculateGaitAmount());
	}

	Super::PhysNavWalking(DeltaTime, Iterations);
}

void UAlsCharacterMovementComponent::PhysCustom(const float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		Super::PhysCustom(DeltaTime, Iterations);
		return;
	}

	Iterations += 1;
	bJustTeleported = false;

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = FVector::ZeroVector;
	}

	ApplyRootMotionToVelocity(DeltaTime);

	MoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentQuat(), false);

	Super::PhysCustom(DeltaTime, Iterations);
}

FVector UAlsCharacterMovementComponent::ConsumeInputVector()
{
	auto InputVector{Super::ConsumeInputVector()};

	if (bInputBlocked)
	{
		return FVector::ZeroVector;
	}

	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && UAlsUtility::TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// Offset the input vector to keep it relative to the movement base.
		InputVector = (BaseRotationSpeed * GetWorld()->GetDeltaSeconds()).RotateVector(InputVector);
	}

	return InputVector;
}

void UAlsCharacterMovementComponent::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance,
													  FFindFloorResult& OutFloorResult, float SweepRadius,
													  const FHitResult* DownwardSweepResult) const
{
	// TODO Copied with modifications from UCharacterMovementComponent::ComputeFloorDist().
	// TODO After the release of a new engine version, this code should be updated to match the source code.

	// ReSharper disable All

	UE_LOG(LogAls, VeryVerbose, TEXT("[Role:%d] ComputeFloorDist: %s at location %s"), (int32)CharacterOwner->GetLocalRole(), *GetNameSafe(CharacterOwner), *CapsuleLocation.ToString());
	OutFloorResult.Clear();

	float PawnRadius, PawnHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	bool bSkipSweep = false;
	if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
	{
		// Only if the supplied sweep was vertical and downward.
		const bool bIsDownward = GetGravitySpaceZ(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd) > 0;
		const bool bIsVertical = ProjectToGravityFloor(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared() <= UE_KINDA_SMALL_NUMBER;
		if (bIsDownward && bIsVertical)
		{
			// Reject hits that are barely on the cusp of the radius of the capsule
			if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
			{
				// Don't try a redundant sweep, regardless of whether this sweep is usable.
				bSkipSweep = true;

				const bool bIsWalkable = IsWalkable(*DownwardSweepResult);
				const float FloorDist = UE_REAL_TO_FLOAT(GetGravitySpaceZ(CapsuleLocation - DownwardSweepResult->Location));
				OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);

				if (bIsWalkable)
				{
					// Use the supplied downward sweep as the floor hit result.
					return;
				}
			}
		}
	}

	// We require the sweep distance to be >= the line distance, otherwise the HitResult can't be interpreted as the sweep result.
	if (SweepDistance < LineDistance)
	{
		ensure(SweepDistance >= LineDistance);
		return;
	}

	bool bBlockingHit = false;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeFloorDist), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

	// Sweep test
	if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
	{
		// Use a shorter height to avoid sweeps giving weird results if we start on a surface.
		// This also allows us to adjust out of penetrations.
		const float ShrinkScale = 0.9f;
		const float ShrinkScaleOverlap = 0.1f;
		float ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale);
		float TraceDist = SweepDistance + ShrinkHeight;
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight);

		FHitResult Hit(1.f);
		bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + TraceDist * GetGravityDirection(), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

		// TODO Start of custom ALS code block.

		const_cast<ThisClass*>(this)->SavePenetrationAdjustment(Hit);

		// TODO End of custom ALS code block.

		if (bBlockingHit)
		{
			// Reject hits adjacent to us, we only care about hits on the bottom portion of our capsule.
			// Check 2D distance to impact point, reject if within a tolerance from radius.
			if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
			{
				// Use a capsule with a slightly smaller radius and shorter height to avoid the adjacent object.
				// Capsule must not be nearly zero or the trace will fall back to a line trace from the start point and have the wrong length.
				CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - SWEEP_EDGE_REJECT_DISTANCE - UE_KINDA_SMALL_NUMBER);
				if (!CapsuleShape.IsNearlyZero())
				{
					ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
					TraceDist = SweepDistance + ShrinkHeight;
					CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
					Hit.Reset(1.f, false);

					bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + TraceDist * GetGravityDirection(), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);
				}
			}

			// Reduce hit distance by ShrinkHeight because we shrank the capsule for the trace.
			// We allow negative distances here, because this allows us to pull out of penetrations.
			const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
			const float SweepResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

			OutFloorResult.SetFromSweep(Hit, SweepResult, false);
			if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
			{
				if (SweepResult <= SweepDistance)
				{
					// Hit within test distance.
					OutFloorResult.bWalkableFloor = true;
					return;
				}
			}
		}
	}

	// Since we require a longer sweep than line trace, we don't want to run the line trace if the sweep missed everything.
	// We do however want to try a line trace if the sweep was stuck in penetration.
	if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
	{
		OutFloorResult.FloorDist = SweepDistance;
		return;
	}

	// Line trace
	if (LineDistance > 0.f)
	{
		const float ShrinkHeight = PawnHalfHeight;
		const FVector LineTraceStart = CapsuleLocation;
		const float TraceDist = LineDistance + ShrinkHeight;
		const FVector Down = TraceDist * GetGravityDirection();
		QueryParams.TraceTag = SCENE_QUERY_STAT_NAME_ONLY(FloorLineTrace);

		FHitResult Hit(1.f);
		bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			if (Hit.Time > 0.f)
			{
				// Reduce hit distance by ShrinkHeight because we started the trace higher than the base.
				// We allow negative distances here, because this allows us to pull out of penetrations.
				const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
				const float LineResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

				OutFloorResult.bBlockingHit = true;
				if (LineResult <= LineDistance && IsWalkable(Hit))
				{
					OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);
					return;
				}
			}
		}
	}

	// No hits were acceptable.
	OutFloorResult.bWalkableFloor = false;

	// ReSharper restore All
}

void UAlsCharacterMovementComponent::PerformMovement(const float DeltaTime)
{
	Super::PerformMovement(DeltaTime);

	// Update the ServerLastTransformUpdateTimeStamp when the control rotation
	// changes. This is required for the view network smoothing to work properly.

	const auto* Controller{HasValidData() ? CharacterOwner->GetController() : nullptr};

	if (Controller != nullptr && IsValid(Controller) && CharacterOwner->GetLocalRole() >= ROLE_Authority &&
		PreviousControlRotation != Controller->GetControlRotation())
	{
		if (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
		{
			ServerLastTransformUpdateTimeStamp = GetPredictionData_Server_Character()->ServerAccumulatedClientTimeStamp;
		}
		else
		{
			ServerLastTransformUpdateTimeStamp = GetWorld()->GetTimeSeconds();
		}
	}
}

FNetworkPredictionData_Client* UAlsCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		auto* MutableThis{const_cast<ThisClass*>(this)};

		MutableThis->ClientPredictionData = new FAlsNetworkPredictionData{*this};
	}

	return ClientPredictionData;
}

void UAlsCharacterMovementComponent::SmoothClientPosition(const float DeltaTime)
{
	auto* PredictionData{GetPredictionData_Client_Character()};
	const auto* Mesh{HasValidData() ? CharacterOwner->GetMesh() : nullptr};

	if (PredictionData != nullptr && Mesh != nullptr && IsValid(Mesh) && Mesh->IsUsingAbsoluteRotation())
	{
		// Calling Super::SmoothClientPosition() will change the mesh's rotation, which is undesirable when using
		// absolute mesh rotation since we're manually updating the mesh's rotation from the animation instance. So,
		// to keep the rotation unchanged, we simply override the predicted rotations with the mesh's current rotation.

		const auto Rotation{Mesh->GetComponentQuat() * CharacterOwner->GetBaseRotationOffset().Inverse()};

		PredictionData->OriginalMeshRotationOffset = Rotation;
		PredictionData->MeshRotationOffset = Rotation;
		PredictionData->MeshRotationTarget = Rotation;
	}

	Super::SmoothClientPosition(DeltaTime);
}

void UAlsCharacterMovementComponent::MoveAutonomous(const float ClientTimeStamp, const float DeltaTime,
													const uint8 CompressedFlags, const FVector& NewAcceleration)
{
	const auto* MoveData{static_cast<FAlsCharacterNetworkMoveData*>(GetCurrentNetworkMoveData())};
	if (MoveData != nullptr)
	{
		RotationMode = MoveData->RotationMode;
		Stance = MoveData->Stance;
		MaxAllowedGait = MoveData->MaxAllowedGait;

		RefreshGaitSettings();
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAcceleration);

	// Process view network smoothing on the listen server.

	const auto* Controller{HasValidData() ? CharacterOwner->GetController() : nullptr};

	if (Controller != nullptr && IsValid(Controller) && IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		const auto NewControlRotation{Controller->GetControlRotation()};

		auto* Character{Cast<AAlsCharacter>(CharacterOwner)};
		if (IsValid(Character))
		{
			Character->CorrectViewNetworkSmoothing(NewControlRotation, false);
		}

		PreviousControlRotation = NewControlRotation;
	}
}

void UAlsCharacterMovementComponent::SavePenetrationAdjustment(const FHitResult& Hit)
{
	if (Hit.bStartPenetrating)
	{
		PendingPenetrationAdjustment = Hit.Normal * Hit.PenetrationDepth;
	}
}

void UAlsCharacterMovementComponent::ApplyPendingPenetrationAdjustment()
{
	if (PendingPenetrationAdjustment.IsNearlyZero())
	{
		return;
	}

	ResolvePenetration(ConstrainDirectionToPlane(PendingPenetrationAdjustment),
					   CurrentFloor.HitResult, UpdatedComponent->GetComponentQuat());

	PendingPenetrationAdjustment = FVector::ZeroVector;
}

void UAlsCharacterMovementComponent::SetMovementSettings(UAlsMovementSettings* NewMovementSettings)
{
	ALS_ENSURE(IsValid(NewMovementSettings));

	MovementSettings = NewMovementSettings;

	RefreshGaitSettings();
}

void UAlsCharacterMovementComponent::RefreshGaitSettings()
{
	if (ALS_ENSURE(IsValid(MovementSettings)))
	{
		const auto* StanceSettings{MovementSettings->RotationModes.Find(RotationMode)};
		const auto* NewGaitSettings{ALS_ENSURE(StanceSettings != nullptr) ? StanceSettings->Stances.Find(Stance) : nullptr};

		GaitSettings = ALS_ENSURE(NewGaitSettings != nullptr) ? *NewGaitSettings : FAlsMovementGaitSettings{};
	}

	RefreshMaxWalkSpeed();
}

void UAlsCharacterMovementComponent::SetRotationMode(const FGameplayTag& NewRotationMode)
{
	if (RotationMode != NewRotationMode)
	{
		RotationMode = NewRotationMode;

		RefreshGaitSettings();
	}
}

void UAlsCharacterMovementComponent::SetStance(const FGameplayTag& NewStance)
{
	if (Stance != NewStance)
	{
		Stance = NewStance;

		RefreshGaitSettings();
	}
}

void UAlsCharacterMovementComponent::SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait)
{
	if (MaxAllowedGait != NewMaxAllowedGait)
	{
		MaxAllowedGait = NewMaxAllowedGait;

		RefreshMaxWalkSpeed();
	}
}

void UAlsCharacterMovementComponent::RefreshMaxWalkSpeed()
{
	MaxWalkSpeed = GaitSettings.GetSpeedByGait(MaxAllowedGait);
	MaxWalkSpeedCrouched = MaxWalkSpeed;
}

float UAlsCharacterMovementComponent::CalculateGaitAmount() const
{
	// Map the character's current speed to the configured movement speeds ranging from 0 to 3,
	// where 0 is stopped, 1 is walking, 2 is running, and 3 is sprinting. This allows us to vary
	// movement speeds but still use the mapped range in calculations for consistent results.

	const auto Speed{UE_REAL_TO_FLOAT(Velocity.Size2D())};

	if (Speed <= GaitSettings.WalkSpeed)
	{
		static const FVector2f GaitAmount{0.0f, 1.0f};

		return FMath::GetMappedRangeValueClamped({0.0f, GaitSettings.WalkSpeed}, GaitAmount, Speed);
	}

	if (Speed <= GaitSettings.RunSpeed)
	{
		static const FVector2f GaitAmount{1.0f, 2.0f};

		return FMath::GetMappedRangeValueClamped({GaitSettings.WalkSpeed, GaitSettings.RunSpeed}, GaitAmount, Speed);
	}

	static const FVector2f GaitAmount{2.0f, 3.0f};

	return FMath::GetMappedRangeValueClamped({GaitSettings.RunSpeed, GaitSettings.SprintSpeed}, GaitAmount, Speed);
}

void UAlsCharacterMovementComponent::SetMovementModeLocked(const bool bNewMovementModeLocked)
{
	bMovementModeLocked = bNewMovementModeLocked;
}

void UAlsCharacterMovementComponent::SetInputBlocked(const bool bNewInputBlocked)
{
	bInputBlocked = bNewInputBlocked;
}

bool UAlsCharacterMovementComponent::TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity)
{
	if (!bPrePenetrationAdjustmentVelocityValid)
	{
		OutVelocity = FVector::ZeroVector;
		return false;
	}

	OutVelocity = PrePenetrationAdjustmentVelocity;

	PrePenetrationAdjustmentVelocity = FVector::ZeroVector;
	bPrePenetrationAdjustmentVelocityValid = false;

	return true;
}

void UAlsCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanCrouchInCurrentState())
	{
		return;
	}

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == GetCrouchedHalfHeight())
	{
		if (!bClientSimulation)
		{
			CharacterOwner->bIsCrouched = true;
		}
		CharacterOwner->OnStartCrouch(0.f, 0.f);
		return;
	}

	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// restore collision size before crouching
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		bShrinkProxyCapsule = true;
	}

	// Change collision size to crouching dimensions
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, GetCrouchedHalfHeight());
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{
		CharacterOwner->bIsCrouched = true;
	}

	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedCrouchedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	CharacterOwner->OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset -= FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UAlsCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			CharacterOwner->bIsCrouched = false;
		}
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		return;
	}

	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Grow to uncrouched size.
	check(CharacterOwner->GetCapsuleComponent());

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							//UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				//UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		CharacterOwner->bIsCrouched = false;
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	AdjustProxyCapsuleSize();
	CharacterOwner->OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset += FVector(0.f, 0.f, ScaledHalfHeightAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UAlsCharacterMovementComponent::Lie(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanLieInCurrentState())
	{
		return;
	}

	// See if collision is already at desired size.
	if (GetAlsCharacter()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == GetCrouchedHalfHeight())
	{
		if (!bClientSimulation)
		{
			GetAlsCharacter()->SetIsLied(true);
		}
		GetAlsCharacter()->OnStartLie(0.f, 0.f);
		return;
	}

	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// restore collision size before crouching
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		bShrinkProxyCapsule = true;
	}

	// Change collision size to crouching dimensions
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, GetCrouchedHalfHeight());
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{
		GetAlsCharacter()->SetIsLied(true);
	}

	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedCrouchedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	GetAlsCharacter()->OnStartLie(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset -= FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UAlsCharacterMovementComponent::UnLie(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	AAlsCharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<AAlsCharacter>();

	// See if collision is already at desired size.
	if (GetAlsCharacter()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			GetAlsCharacter()->SetIsLied(false);
		}
		GetAlsCharacter()->OnEndLie(0.f, 0.f);
		return;
	}

	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Grow to uncrouched size.
	check(CharacterOwner->GetCapsuleComponent());

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		GetAlsCharacter()->SetIsLied(false);
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	AdjustProxyCapsuleSize();
	GetAlsCharacter()->OnEndLie(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset += FVector(0.f, 0.f, ScaledHalfHeightAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

TObjectPtr<AAlsCharacter> UAlsCharacterMovementComponent::GetAlsCharacter() const
{
	return Cast<AAlsCharacter>(CharacterOwner);
}

void UAlsCharacterMovementComponent::UpdateCapsuleSize(float DeltaTime, float TargetHalfHeight, float HeightSpeed, float TargetRadius, float RadiusSpeed)
{
	if (!HasValidData())
	{
		return;
	}

	check(CharacterOwner->GetCapsuleComponent());

	TargetRadius = FMath::Max(0.f, TargetRadius);
	TargetHalfHeight = FMath::Max3(0.f, TargetRadius, TargetHalfHeight);

	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float HalfHeight = FMath::FInterpConstantTo(OldUnscaledHalfHeight, TargetHalfHeight, DeltaTime, HeightSpeed);
	const float Radius = FMath::FInterpConstantTo(OldUnscaledRadius, TargetRadius, DeltaTime, RadiusSpeed);
	
	if (OldUnscaledHalfHeight != HalfHeight || OldUnscaledRadius != Radius)
	{
		// Now call SetCapsuleSize() to cause touch/untouch events and actually grow the capsule
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(Radius, HalfHeight, false);

		auto* Mesh{HasValidData() ? CharacterOwner->GetMesh() : nullptr};

		if (Mesh != nullptr && IsValid(Mesh))
		{
			Mesh->GetRelativeLocation_DirectMutable().Z = -HalfHeight;
		}

		// Change actor location must be applied after change mesh relative location.
		CharacterOwner->AddActorLocalOffset(FVector{0.0, 0.0, HalfHeight - OldUnscaledHalfHeight});
	}
}

bool UAlsCharacterMovementComponent::CanAttemptJump() const
{
	return !bWantsToLie && Super::CanAttemptJump();
}

bool UAlsCharacterMovementComponent::IsLying() const
{
	auto AlsCharacter{GetAlsCharacter()};
	return AlsCharacter && AlsCharacter->IsLied();
}

bool UAlsCharacterMovementComponent::CanLieInCurrentState() const
{
	if (!CanEverLie())
	{
		return false;
	}

	return (IsFalling() || IsMovingOnGround()) && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UAlsCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	// Proxies get replicated crouch state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Check for a change in lie state. Players toggle lying by changing bWantsToLie.
		const bool bIsLying = IsLying();
		if (bIsLying && (!bWantsToLie || !CanLieInCurrentState()))
		{
			UnLie(false);
		}
		else if (!bIsLying && bWantsToLie && CanLieInCurrentState())
		{
			Lie(false);
		}
	}
}

void UAlsCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	// Proxies get replicated lie state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Unlie if no longer allowed to be lied
		if (IsLying() && !CanLieInCurrentState())
		{
			UnLie(false);
		}
	}
}

FName FAlsCharacterMovementComponentAsyncCallback::GetFNameForStatId() const
{
	const static FLazyName StaticName("FAlsCharacterMovementComponentAsyncCallback");
	return StaticName;
}

void FAlsCharacterMovementComponentAsyncCallback::OnPreSimulate_Internal()
{
	PreSimulateImpl<FAlsCharacterMovementComponentAsyncInput, FAlsCharacterMovementComponentAsyncOutput>(*this);
}

void UAlsCharacterMovementComponent::FillAsyncInput(const FVector& InputVector, FCharacterMovementComponentAsyncInput& AsyncInput)
{
	if (!CharacterOwner || !CharacterOwner->Controller)
	{
		return;
	}

	auto& AlsAsyncInput{static_cast<FAlsCharacterMovementComponentAsyncInput&>(AsyncInput)};

	AlsAsyncInput.bCanEverCrouch = CanEverCrouch();

	// Only game thread inputs need to be updated here.
	AlsAsyncInput.GTInputs.bWantsToCrouch = bWantsToCrouch;

	auto* AlsAsyncSimState{static_cast<FAlsCharacterMovementComponentAsyncOutput*>(AsyncSimState.Get())};

	AlsAsyncSimState->RotationMode = RotationMode;
	AlsAsyncSimState->Stance = Stance;
	AlsAsyncSimState->MaxAllowedGait = MaxAllowedGait;
	AlsAsyncSimState->bWantsToCrouch = bWantsToCrouch;
	AlsAsyncSimState->bIsCrouched = CharacterOwner->bIsCrouched;
}

void UAlsCharacterMovementComponent::BuildAsyncInput()
{
	if (CharacterMovementCVars::AsyncCharacterMovement == 1 && IsAsyncCallbackRegistered())
	{
		FAlsCharacterMovementComponentAsyncInput* Input = AlsAsyncCallback->GetProducerInputData_External();
		if (Input->bInitialized == false)
		{
			Input->Initialize<FAlsCharacterMovementComponentAsyncInput::FCharacterInput, FAlsCharacterMovementComponentAsyncInput::FUpdatedComponentInput>();
		}

		if (AsyncSimState.IsValid() == false)
		{
			AsyncSimState = MakeShared<FAlsCharacterMovementComponentAsyncOutput, ESPMode::ThreadSafe>();
		}
		Input->AsyncSimState = AsyncSimState;

		const FVector InputVector = ConsumeInputVector();
		FillAsyncInput(InputVector, *Input);

		PostBuildAsyncInput();
	}
}

void UAlsCharacterMovementComponent::ApplyAsyncOutput(FCharacterMovementComponentAsyncOutput& Output)
{
	Super::ApplyAsyncOutput(Output);

	ensure(Output.DeltaTime > 0.0f);

	if (Output.IsValid() == false)
	{
		return;
	}

	auto& AlsOutput{static_cast<FAlsCharacterMovementComponentAsyncOutput&>(Output)};

	// TODO Crouching, check if scaled radius/half haeight changed on capsule and handle?
	ensure(AlsOutput.bIsLied == false);
	RotationMode = AlsOutput.RotationMode;
	Stance = AlsOutput.Stance;
	MaxAllowedGait = AlsOutput.MaxAllowedGait;
	bWantsToLie = AlsOutput.bWantsToLie;
}

void UAlsCharacterMovementComponent::ProcessAsyncOutput()
{
	if (CharacterMovementCVars::AsyncCharacterMovement == 1 && IsAsyncCallbackRegistered())
	{
		while (auto Output = AlsAsyncCallback->PopOutputData_External())
		{
			ApplyAsyncOutput(*Output);
		}
	}
}

void UAlsCharacterMovementComponent::RegisterAsyncCallback()
{
	if (CharacterMovementCVars::AsyncCharacterMovement == 1)
	{
		if (UWorld* World = GetWorld())
		{
			if (FPhysScene* PhysScene = World->GetPhysicsScene())
			{
				AlsAsyncCallback = PhysScene->GetSolver()->CreateAndRegisterSimCallbackObject_External<FAlsCharacterMovementComponentAsyncCallback>();
			}
		}
	}
}

bool UAlsCharacterMovementComponent::IsAsyncCallbackRegistered() const
{
	return AlsAsyncCallback != nullptr;
}
