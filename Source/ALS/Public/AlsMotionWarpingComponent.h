// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionWarpingComponent.h"
#include "AlsMotionWarpingComponent.generated.h"

/** Parameter Structure for RPC.
 * Assuming that bFollowComponent is true.
 */
USTRUCT(BlueprintType)
struct ALS_API FAlsMotionWarpingTargetSmall
{
	GENERATED_BODY()

	/** Unique name for this warp target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FName Name;

	/** When the warp target is created from a component this stores the location of the component at the time of creation, otherwise its the location supplied by the user */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FVector_NetQuantize10 Location;

	/** When the warp target is created from a component this stores the rotation of the component at the time of creation, otherwise its the rotation supplied by the user */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FRotator Rotation;

	/** Optional component used to calculate the final target transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TWeakObjectPtr<const USceneComponent> Component;
};

/**
 * 
 */
UCLASS()
class ALS_API UAlsMotionWarpingComponent : public UMotionWarpingComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Motion Warping")
	void AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation);

	UFUNCTION(BlueprintCallable, Category = "ALS|Motion Warping")
	void AutonomousAddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation);

	UFUNCTION(BlueprintCallable, Category = "ALS|Motion Warping")
	void AddOrUpdateReplicatedWarpTarget(const FMotionWarpingTarget& WarpTarget);

	UFUNCTION(BlueprintCallable, Category = "ALS|Motion Warping")
	void AutonomousAddOrUpdateReplicatedWarpTarget(const FMotionWarpingTarget& WarpTarget);

private:
	UFUNCTION(Server, Reliable)
	void ServerAddOrUpdateWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector_NetQuantize TargetLocation, FRotator TargetRotation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAddOrUpdateWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector_NetQuantize TargetLocation, FRotator TargetRotation);

	UFUNCTION(Server, Reliable)
	void ServerAddOrUpdateWarpTarget(const FAlsMotionWarpingTargetSmall& WarpTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAddOrUpdateWarpTarget(const FAlsMotionWarpingTargetSmall& WarpTarget);
};
