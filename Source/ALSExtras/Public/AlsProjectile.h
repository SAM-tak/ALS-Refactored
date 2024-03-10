// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "AlsProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class ALSEXTRAS_API AAlsProjectile : public AActor
{
	GENERATED_UCLASS_BODY()
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	TObjectPtr<UCapsuleComponent> CollisionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	// 発射物の発射方向の速度を初期化する関数。
	void FireInDirection(const FVector& ShootDirection);
};
