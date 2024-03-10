// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Actions/AlsGameplayAbility_Ragdolling.h"
#include "Abilities/Tasks/AlsAbilityTask_Tick.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAnimationInstance.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsRagdollingAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Ragdolling)

UAlsGameplayAbility_Ragdolling::UAlsGameplayAbility_Ragdolling(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityTags.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::BeingKnockedDown);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Root);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::BeingKnockedDown);

	GroundTraceResponses.WorldStatic = ECR_Block;
	GroundTraceResponses.WorldDynamic = ECR_Block;
	GroundTraceResponses.Destructible = ECR_Block;
}

void UAlsGameplayAbility_Ragdolling::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TargetLocation, Parameters)
}

void UAlsGameplayAbility_Ragdolling::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* AnimInstance{Character->GetAlsAnimationInstace()};

	RagdollingAnimInstance = Character->GetAlsAnimationInstace()->GetRagdollingAnimInstance();

	if (!IsValid(RagdollingAnimInstance))
	{
		return;
	}

	OverrideAnimInstance = Cast<UAlsLinkedAnimationInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverrideAnimLayersClass));

	if (!OverrideAnimInstance.IsValid())
	{
		Character->GetMesh()->LinkAnimClassLayers(OverrideAnimLayersClass);

		OverrideAnimInstance = Cast<UAlsLinkedAnimationInstance>(Character->GetMesh()->GetLinkedAnimLayerInstanceByClass(OverrideAnimLayersClass));
	}

	if(!OverrideAnimInstance.IsValid())
	{
		return;
	}

	bOnGroundedAndAgedFired = false;

	// Ensure freeze flag is off.

	RagdollingAnimInstance->UnFreeze();

	auto* CharacterMovement{Character->GetAlsCharacterMovement()};

	// Initialize bFacingUpward flag by current movement direction. If Velocity is Zero, it is chosen bFacingUpward is true.
	// And determine target yaw angle of the character.

	const auto Direction = CharacterMovement->Velocity.GetSafeNormal2D();

	if (Direction.SizeSquared2D() > 0.0)
	{
		bFacingUpward = Character->GetActorForwardVector().Dot(Direction) < -0.25f;
		LyingDownYawAngleDelta = UAlsMath::DirectionToAngleXY(bFacingUpward ? -Direction : Direction) - Character->GetActorRotation().Yaw;
	}
	else
	{
		bFacingUpward = true;
		LyingDownYawAngleDelta = 0.0;
	}

	// Stop any active montages.

	static constexpr auto BlendOutDuration{0.2f};

	AnimInstance->Montage_Stop(BlendOutDuration);

	// Disable movement corrections and reset network smoothing.

	CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = true;

	// Disable capsule collision. other physics states will be changed by physical aniamtion process

	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Character->IsLocallyControlled() || (Character->GetLocalRole() >= ROLE_Authority && !IsValid(Character->GetController())))
	{
		SetTargetLocation(Character->GetMesh()->GetBoneLocation(UAlsConstants::PelvisBoneName()));
	}

	// Clear the character movement mode and set the locomotion action to ragdolling.

	CharacterMovement->SetMovementMode(MOVE_None);
	CharacterMovement->SetMovementModeLocked(true);

	ElapsedTime = 0.0f;
	TimeAfterGrounded = TimeAfterGroundedAndStopped = 0.0f;
	bFacingUpward = bGrounded = false;
	bPreviousGrounded = true;
	bFreezing = false;
	PrevActorLocation = Character->GetActorLocation();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsActive())
	{
		RagdollingAnimInstance->Refresh(*this, true);
		TickTask = UAlsAbilityTask_Tick::New(this, FName(TEXT("UAlsGameplayAbility_Ragdolling")));
		if (TickTask.IsValid())
		{
			TickTask->OnTick.AddDynamic(this, &ThisClass::Tick);
			TickTask->ReadyForActivation();
		}
	}
}

void UAlsGameplayAbility_Ragdolling::Tick(const float DeltaTime)
{
	if (!IsActive() || bFreezing)
	{
		return;
	}

	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};

	bool bLocallyControlled{Character->IsLocallyControlled() || (Character->GetLocalRole() >= ROLE_Authority && !IsValid(Character->GetController()))};

	if (bLocallyControlled)
	{
		SetTargetLocation(Character->GetMesh()->GetBoneLocation(UAlsConstants::PelvisBoneName()));
	}

	// Just for info.
	CharacterMovement->Velocity = DeltaTime > 0.0f ? (Character->GetActorLocation() - PrevActorLocation) / DeltaTime : FVector::Zero();
	PrevActorLocation = Character->GetActorLocation();

	// Prevent the capsule from going through the ground when the ragdoll is lying on the ground.

	// While we could get rid of the line trace here and just use TargetLocation
	// as the character's location, we don't do that because the camera depends on the
	// capsule's bottom location, so its removal will cause the camera to behave erratically.

	Character->SetActorLocation(TraceGround(), true);

	if (IsGroundedAndAged())
	{
		// Determine whether the ragdoll is facing upward or downward.

		const auto PelvisRotation{Character->GetMesh()->GetBoneTransform(UAlsConstants::PelvisBoneName()).Rotator()};

		const auto PelvisDirDotUp{PelvisRotation.RotateVector(FVector::RightVector).Dot(FVector::UpVector)};

		if (bFacingUpward)
		{
			if (PelvisDirDotUp < -0.5f)
			{
				bFacingUpward = false;
			}
		}
		else
		{
			if (PelvisDirDotUp > 0.5f)
			{
				bFacingUpward = true;
			}
		}
	}

	RagdollingAnimInstance->Refresh(*this, IsActive());

	if (bAllowFreeze)
	{
		RootBoneSpeed = CharacterMovement->Velocity.Size();

		RagdollingAnimInstance->UnFreeze();

		if (bGrounded)
		{
			TimeAfterGrounded += DeltaTime;

			if (TimeAfterGroundedForForceFreezing > 0.0f &&
				TimeAfterGrounded > TimeAfterGroundedForForceFreezing)
			{
				bFreezing = true;
			}
			else if (RootBoneSpeed < RootBoneSpeedConsideredAsStopped)
			{
				TimeAfterGroundedAndStopped += DeltaTime;

				if (TimeAfterGroundedAndStoppedForForceFreezing > 0.0f &&
					TimeAfterGroundedAndStopped > TimeAfterGroundedAndStoppedForForceFreezing)
				{
					bFreezing = true;
				}
				else
				{
					MaxBoneSpeed = 0.0f;
					MaxBoneAngularSpeed = 0.0f;
					Character->GetMesh()->ForEachBodyBelow(UAlsConstants::PelvisBoneName(), true, false, [&](FBodyInstance *Body) {
						float Speed = Body->GetUnrealWorldVelocity().Size();
						if(Speed > MaxBoneSpeed) MaxBoneSpeed = Speed;
						Speed = FMath::RadiansToDegrees(Body->GetUnrealWorldAngularVelocityInRadians().Size());
						if(Speed > MaxBoneAngularSpeed) MaxBoneAngularSpeed = Speed;
					});
					bFreezing = MaxBoneSpeed < SpeedThresholdToFreeze && MaxBoneAngularSpeed < AngularSpeedThresholdToFreeze;
				}
			}
			else
			{
				TimeAfterGroundedAndStopped = 0.0f;
			}

			if (bFreezing)
			{
				RagdollingAnimInstance->Freeze();
				Character->GetMesh()->SetAllBodiesSimulatePhysics(false);
			}
		}
		else
		{
			TimeAfterGrounded = TimeAfterGroundedAndStopped = 0.0f;
		}
	}

	if (ElapsedTime <= StartBlendTime && ElapsedTime + DeltaTime > StartBlendTime)
	{
		// Re-initialize bFacingUpward flag by current movement direction. If Velocity is Zero, it is chosen bFacingUpward is true.
		bFacingUpward = Character->GetActorForwardVector().Dot(CharacterMovement->Velocity.GetSafeNormal2D()) <= 0.0f;
	}

	if (bPreviousGrounded != bGrounded)
	{
		if (bGrounded)
		{
			Character->Crouch();
		}
		else
		{
			Character->UnCrouch();
		}
	}
	bPreviousGrounded = bGrounded;

	K2_OnTick(DeltaTime);

	ElapsedTime += DeltaTime;

	if (IsGroundedAndAged())
	{
		if (!bOnGroundedAndAgedFired)
		{
			bOnGroundedAndAgedFired = true;
			K2_OnGroundedAndAged();
		}
		Character->Lie();
	}
	else
	{
		bOnGroundedAndAgedFired = false;
	}
}

void UAlsGameplayAbility_Ragdolling::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto CharacterMovement{Character->GetAlsCharacterMovement()};

	RagdollingAnimInstance->Freeze();
	RagdollingAnimInstance->Refresh(*this, false);

	// Re-enable capsule collision.

	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = false;

	if (RagdollingAnimInstance && ElapsedTime > StartBlendTime)
	{
		const auto PelvisTransform{Character->GetMesh()->GetBoneTransform(UAlsConstants::PelvisBoneName())};
		const auto PelvisRotation{PelvisTransform.Rotator()};

		// Determine yaw angle of the character.

		auto NewActorRotation{Character->GetActorRotation()};
		NewActorRotation.Yaw = UAlsMath::DirectionToAngleXY(PelvisRotation.RotateVector(
			FMath::Abs(PelvisRotation.RotateVector(FVector::ForwardVector).GetSafeNormal2D().Dot(FVector::UpVector)) > 0.5f ?
			(bFacingUpward ? FVector::RightVector : FVector::LeftVector) :
			(bFacingUpward ? FVector::BackwardVector : FVector::ForwardVector)).GetSafeNormal2D());
		Character->SetActorRotation(NewActorRotation, ETeleportType::TeleportPhysics);

		// Restore the pelvis transform to the state it was in before we changed
		// the character and mesh transforms to keep its world transform unchanged.

		const auto& ReferenceSkeleton{Character->GetMesh()->GetSkeletalMeshAsset()->GetRefSkeleton()};

		const auto PelvisBoneIndex{ReferenceSkeleton.FindBoneIndex(UAlsConstants::PelvisBoneName())};
		auto& FinalRagdollPose{RagdollingAnimInstance->GetFinalPoseSnapshot()};
		if (ALS_ENSURE(PelvisBoneIndex >= 0) && PelvisBoneIndex < FinalRagdollPose.LocalTransforms.Num())
		{
			// We expect the pelvis bone to be the root bone or attached to it, so we can safely use the mesh transform here.
			FinalRagdollPose.LocalTransforms[PelvisBoneIndex] = PelvisTransform.GetRelativeTransform(Character->GetMesh()->GetComponentTransform());
		}
	}

	SetTargetLocation(FVector::ZeroVector);

	// If the ragdoll is on the ground, set the movement mode to walking and play a get up montage. If not, set
	// the movement mode to falling and update the character movement velocity to match the last ragdoll velocity.

	CharacterMovement->SetMovementModeLocked(false);

	if (bGrounded)
	{
		CharacterMovement->SetMovementMode(MOVE_Walking);
	}
	else
	{
		CharacterMovement->SetMovementMode(MOVE_Falling);
	}

	if (IsGroundedAndAged())
	{
		auto* AbilitySystem{GetAlsAbilitySystemComponentFromActorInfo()};
		AbilitySystem->SetLooseGameplayTagCount(AlsStateFlagTags::FacingUpward, bFacingUpward ? 1 : 0);
	}

	Character->GetMesh()->UnlinkAnimClassLayers(OverrideAnimLayersClass);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAlsGameplayAbility_Ragdolling::SetTargetLocation(const FVector& NewTargetLocation)
{
	if (TargetLocation != NewTargetLocation)
	{
		TargetLocation = NewTargetLocation;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TargetLocation, this)

		auto* Character{GetAlsCharacterFromActorInfo()};
		if (Character->GetLocalRole() == ROLE_AutonomousProxy)
		{
			ServerSetTargetLocation(TargetLocation);
		}
	}
}

void UAlsGameplayAbility_Ragdolling::ServerSetTargetLocation_Implementation(const FVector_NetQuantize& NewTargetLocation)
{
	SetTargetLocation(NewTargetLocation);
}

FVector UAlsGameplayAbility_Ragdolling::TraceGround()
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};

	const auto CapsuleHalfHeight{Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};

	const auto TraceStart{!TargetLocation.IsZero() ? FVector{TargetLocation} : Character->GetActorLocation()};
	const FVector TraceEnd{TraceStart.X, TraceStart.Y, TraceStart.Z - CapsuleHalfHeight};

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, GroundTraceChannel, {__FUNCTION__, false, Character}, GroundTraceResponses);

	bGrounded = CharacterMovement->IsWalkable(Hit);

	return {
		TraceStart.X, TraceStart.Y,
		bGrounded ? Hit.ImpactPoint.Z + CapsuleHalfHeight + UCharacterMovementComponent::MIN_FLOOR_DIST : TraceStart.Z
	};
}

bool UAlsGameplayAbility_Ragdolling::IsGroundedAndAged() const
{
	return bGrounded && ElapsedTime > StartBlendTime;
}

void UAlsGameplayAbility_Ragdolling::RequestCancel()
{
	bCancelRequested = true;
}

void UAlsGameplayAbility_Ragdolling::Cancel()
{
	bCancelRequested = false;
	CancelAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

#if WITH_EDITOR
void UAlsGameplayAbility_Ragdolling::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(ThisClass, GroundTraceResponseChannels))
	{
		return;
	}

	GroundTraceResponses.SetAllChannels(ECR_Ignore);

	for (const auto& CollisionChannel : GroundTraceResponseChannels)
	{
		GroundTraceResponses.SetResponse(CollisionChannel, ECR_Block);
	}
}
#endif
