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

void UAlsMotionWarpingComponent::AddOrUpdateReplicatedWarpTargetFromComponent(FName WarpTargetName, const USceneComponent* Component, FName BoneName,
																			  bool bFollowComponent)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	AddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);

	if (Character->HasServerRole())
	{
		MulticastAddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);
	}
}

void UAlsMotionWarpingComponent::AutonomousAddOrUpdateReplicatedWarpTargetFromComponent(FName WarpTargetName, const USceneComponent* Component, FName BoneName,
																						bool bFollowComponent)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	AddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);

	if (Character->IsCharacterSelf())
	{
		ServerAddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);
	}
}

void UAlsMotionWarpingComponent::ServerAddOrUpdateWarpTargetFromComponent_Implementation(FName WarpTargetName, const USceneComponent* Component, FName BoneName,
																						 bool bFollowComponent)
{
	AddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);

	MulticastAddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);
}

void UAlsMotionWarpingComponent::MulticastAddOrUpdateWarpTargetFromComponent_Implementation(FName WarpTargetName, const USceneComponent* Component, FName BoneName,
																							bool bFollowComponent)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	if (Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		AddOrUpdateWarpTargetFromComponent(WarpTargetName, Component, BoneName, bFollowComponent);
	}
}
