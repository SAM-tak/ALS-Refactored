// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/AlsGameplayAbility_Rolling.h"
#include "Abilities/Tasks/AlsAbilityTask_Tick.h"
#include "AlsCharacter.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Utility/AlsGameplayTags.h"
#include "Utility/AlsMath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility_Rolling)

UAlsGameplayAbility_Rolling::UAlsGameplayAbility_Rolling(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	AbilityTags.AddTag(AlsLocomotionActionTags::Rolling);
	ActivationOwnedTags.AddTag(AlsLocomotionActionTags::Rolling);
	CancelAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Root);
	BlockAbilitiesWithTag.AddTag(AlsLocomotionActionTags::Rolling);
}

float UAlsGameplayAbility_Rolling::CalcTargetYawAngle_Implementation() const
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	return bRotateToInputOnStart && Character->GetLocomotionState().bHasInput
		? Character->GetLocomotionState().InputYawAngle
		: UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw));
}

bool UAlsGameplayAbility_Rolling::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
													 const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
													 OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		const auto* Character{Cast<AAlsCharacter>(ActorInfo->OwnerActor)};
		return IsValid(Character) && Character->HasMatchingGameplayTag(AlsLocomotionModeTags::Grounded);
	}
	return false;
}

void UAlsGameplayAbility_Rolling::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	auto* Character{GetAlsCharacterFromActorInfo()};

	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetCharacterMovement()->FlushServerMoves();

		const auto InitialYawAngle{UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw))};
		Character->RefreshRotationInstant(InitialYawAngle);
	}

	TargetYawAngle = CalcTargetYawAngle();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsActive())
	{
		TickTask = UAlsAbilityTask_Tick::New(this, FName(TEXT("UAlsGameplayAbility_Rolling")));
		if (TickTask.IsValid())
		{
			TickTask->OnTick.AddDynamic(this, &ThisClass::ProcessTick);
			TickTask->ReadyForActivation();
		}

		if (Character->GetLocalRole() <= ROLE_SimulatedProxy ||
			Character->GetMesh()->GetAnimInstance()->RootMotionMode <= ERootMotionMode::IgnoreRootMotion)
		{
			PhysicsRotationHandle.Reset();
		}
		else
		{
			PhysicsRotationHandle = Character->GetAlsCharacterMovement()->OnPhysicsRotation.AddUObject(this, &ThisClass::RefreshRolling);
		}
	}
}

void UAlsGameplayAbility_Rolling::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	auto* Character{GetAlsCharacterFromActorInfo()};

	if (PhysicsRotationHandle.IsValid())
	{
		Character->GetAlsCharacterMovement()->OnPhysicsRotation.Remove(PhysicsRotationHandle);
		PhysicsRotationHandle.Reset();
	}

	if (bCrouchOnStart)
	{
		if (Character->GetDesiredStance() != AlsDesiredStanceTags::Crouching)
		{
			Character->UnCrouch();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAlsGameplayAbility_Rolling::Tick_Implementation(const float DeltaTime)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto* AnimInstance{Character->GetMesh()->GetAnimInstance()};
	if (!PhysicsRotationHandle.IsValid())
	{
		// Refresh rolling physics here because AAlsCharacter::PhysicsRotation()
		// won't be called on simulated proxies or with ignored root motion.

		RefreshRolling(DeltaTime);
	}

	if (bCrouchOnStart)
	{
		Character->Crouch();
	}

	if (bCancelRollingWhenInAir)
	{
		if(Character->HasMatchingGameplayTag(AlsLocomotionModeTags::InAir))
		{
			if (InAirTime >= TimeToCancel)
			{
				EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(),
						   ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateNo, true);

				if (TryActiveWhenCancel.IsValid())
				{
					GetAlsAbilitySystemComponentFromActorInfo()->TryActivateAbilitiesBySingleTag(TryActiveWhenCancel);
				}
			}
			else
			{
				InAirTime += DeltaTime;
			}
		}
		else if (InAirTime > 0.0f)
		{
			InAirTime = 0.0f;
		}
	}
}

void UAlsGameplayAbility_Rolling::ProcessTick(const float DeltaTime)
{
	Tick(DeltaTime);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UAlsGameplayAbility_Rolling::RefreshRolling(const float DeltaTime)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	auto TargetRotation{Character->GetCharacterMovement()->UpdatedComponent->GetComponentRotation()};

	if (RotationInterpolationSpeed <= 0.0f)
	{
		TargetRotation.Yaw = TargetYawAngle;

		Character->GetCharacterMovement()->MoveUpdatedComponent(FVector::ZeroVector, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		TargetRotation.Yaw = UAlsMath::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(TargetRotation.Yaw)),
		                                                     TargetYawAngle, DeltaTime,
		                                                     RotationInterpolationSpeed);

		Character->GetCharacterMovement()->MoveUpdatedComponent(FVector::ZeroVector, TargetRotation, false);
	}
}
