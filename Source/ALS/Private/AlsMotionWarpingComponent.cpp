#include "AlsMotionWarpingComponent.h"

#include "AlsCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsMotionWarpingComponent)

void UAlsMotionWarpingComponent::AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);

	if (Character->HasServerRole())
	{
		MulticastAddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
	}
}

void UAlsMotionWarpingComponent::AutonomousAddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);

	if (Character->IsCharacterSelf())
	{
		ServerAddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
	}
}

void UAlsMotionWarpingComponent::ServerAddOrUpdateWarpTargetFromLocationAndRotation_Implementation(FName WarpTargetName, FVector_NetQuantize TargetLocation,
																								   FRotator TargetRotation)
{
	AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);

	MulticastAddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
}

void UAlsMotionWarpingComponent::MulticastAddOrUpdateWarpTargetFromLocationAndRotation_Implementation(FName WarpTargetName, FVector_NetQuantize TargetLocation,
																									  FRotator TargetRotation)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	if (Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
	}
}

void UAlsMotionWarpingComponent::AddOrUpdateReplicatedWarpTarget(const FMotionWarpingTarget& WarpTarget)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	AddOrUpdateWarpTarget(WarpTarget);

	if (Character->HasServerRole())
	{
		FAlsMotionWarpingTargetSmall MotionWarpingTargetSmall{
			WarpTarget.Name,
			WarpTarget.Location,
			WarpTarget.Rotation,
			WarpTarget.Component
		};
		MulticastAddOrUpdateWarpTarget(MotionWarpingTargetSmall);
	}
}

void UAlsMotionWarpingComponent::AutonomousAddOrUpdateReplicatedWarpTarget(const FMotionWarpingTarget& WarpTarget)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	AddOrUpdateWarpTarget(WarpTarget);

	if (Character->IsCharacterSelf())
	{
		FAlsMotionWarpingTargetSmall MotionWarpingTargetSmall{
			WarpTarget.Name,
			WarpTarget.Location,
			WarpTarget.Rotation,
			WarpTarget.Component
		};
		ServerAddOrUpdateWarpTarget(MotionWarpingTargetSmall);
	}
}

void UAlsMotionWarpingComponent::ServerAddOrUpdateWarpTarget_Implementation(const FAlsMotionWarpingTargetSmall& WarpTarget)
{
	FMotionWarpingTarget MotionWarpingTarget;
	MotionWarpingTarget.Name = WarpTarget.Name;
	MotionWarpingTarget.Location = WarpTarget.Location;
	MotionWarpingTarget.Rotation = WarpTarget.Rotation;
	MotionWarpingTarget.Component = WarpTarget.Component;
	MotionWarpingTarget.bFollowComponent = true;
	AddOrUpdateWarpTarget(MotionWarpingTarget);

	MulticastAddOrUpdateWarpTarget(WarpTarget);
}

void UAlsMotionWarpingComponent::MulticastAddOrUpdateWarpTarget_Implementation(const FAlsMotionWarpingTargetSmall& WarpTarget)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	if (Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		FMotionWarpingTarget MotionWarpingTarget;
		MotionWarpingTarget.Name = WarpTarget.Name;
		MotionWarpingTarget.Location = WarpTarget.Location;
		MotionWarpingTarget.Rotation = WarpTarget.Rotation;
		MotionWarpingTarget.Component = WarpTarget.Component;
		MotionWarpingTarget.bFollowComponent = true;
		AddOrUpdateWarpTarget(MotionWarpingTarget);
	}
}
