// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/AlsGameplayAbility.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "AlsAbilitySystemComponent.h"
#include "AlsMotionWarpingComponent.h"
#include "Engine/InputDelegateBinding.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayAbility)

UAlsGameplayAbility::UAlsGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AAlsCharacter* UAlsGameplayAbility::GetAlsCharacterFromActorInfo() const
{
	if (!ensure(CurrentActorInfo))
	{
		return nullptr;
	}
	return Cast<AAlsCharacter>(CurrentActorInfo->OwnerActor.Get());
}

UAlsAbilitySystemComponent* UAlsGameplayAbility::GetAlsAbilitySystemComponentFromActorInfo() const
{
	auto* AlsCharacter{GetAlsCharacterFromActorInfo()};
	return AlsCharacter ? AlsCharacter->GetAlsAbilitySystem() : nullptr;
}

UWorld* UAlsGameplayAbility::GetWorld() const
{
	auto* RetVal{Super::GetWorld()};
	if (IsValid(RetVal))
	{
		return RetVal;
	}
	else if (CurrentActorInfo)
	{
		if (CurrentActorInfo->AvatarActor.IsValid())
		{
			return CurrentActorInfo->AvatarActor->GetWorld();
		}
		if (CurrentActorInfo->OwnerActor.IsValid())
		{
			return CurrentActorInfo->OwnerActor->GetWorld();
		}
	}
	return nullptr;
}

void UAlsGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
										  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		BindInput(ActorInfo->OwnerActor->InputComponent.Get());
	}
}

void UAlsGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		UnbindInput(ActorInfo->OwnerActor->InputComponent.Get());
	}

	if (bStopCurrentMontageOnEndAbility)
	{
		StopCurrentMontage(OverrideBlendOutTimeOnEndAbility);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UAlsGameplayAbility::PlayMontage(const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayAbilityActorInfo* ActorInfo,
									  const FAlsPlayMontageParameter& Parameter)
{
	auto* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();

	if (Parameter.MontageToPlay && AbilitySystemComponent)
	{
		if (AbilitySystemComponent->PlayMontage(this, ActivationInfo, Parameter.MontageToPlay, Parameter.PlayRate, Parameter.SectionName, Parameter.StartTime))
		{
			return true;
		}
	}
	return false;
}

bool UAlsGameplayAbility::PlayMontage(const FAlsPlayMontageParameter& Parameter)
{
	return PlayMontage(GetCurrentActivationInfo(), GetCurrentActorInfo(), Parameter);
}

void UAlsGameplayAbility::StopCurrentMontage(const FGameplayAbilityActorInfo* ActorInfo, float OverrideBlendOutTime) const
{
	if (IsValid(GetCurrentMontage()))
	{
		auto* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->StopMontageIfCurrent(*GetCurrentMontage(), OverrideBlendOutTime);
		}
	}
}

void UAlsGameplayAbility::StopCurrentMontage(float OverrideBlendOutTime) const
{
	StopCurrentMontage(GetCurrentActorInfo(), OverrideBlendOutTime);
}

void UAlsGameplayAbility::AddOrUpdateWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetMotionWarping()->AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(WarpTargetName, TargetLocation, TargetRotation);
	}
}

void UAlsGameplayAbility::AddOrUpdateWarpTarget(const FMotionWarpingTarget& WarpTarget)
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetMotionWarping()->AddOrUpdateReplicatedWarpTarget(WarpTarget);
	}
}

void UAlsGameplayAbility::SetInputBlocked(bool bBlocked) const
{
	auto* Character{GetAlsCharacterFromActorInfo()};
	if (Character->GetLocalRole() < ROLE_Authority)
	{
		Character->GetAlsCharacterMovement()->SetInputBlocked(bBlocked);
	}
}

void UAlsGameplayAbility::OnControllerChanged(AController* PreviousController, AController* NewController)
{
	if (IsValid(PreviousController) && IsValid(PreviousController->InputComponent))
	{
		UnbindInput(PreviousController->InputComponent);
	}
	if (IsValid(NewController) && IsValid(NewController->InputComponent))
	{
		BindInput(NewController->InputComponent);
	}
}

void UAlsGameplayAbility::BindInput(UInputComponent* InputComponent)
{
	if(bEnableInputBinding && !bInputBinded && IsValid(InputComponent))
	{
		UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent, this);
		bInputBinded = true;
	}
}

void UAlsGameplayAbility::UnbindInput(UInputComponent* InputComponent)
{
	if(bInputBinded && IsValid(InputComponent))
	{
		InputComponent->ClearBindingsForObject(this);
	}
	bInputBinded = false;
}
