#include "AlsAbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "Abilities/AlsGameplayAbility.h"
#include "Animation/AnimMontage.h"
#include "RootMotionSources/AlsRootMotionSource_Mantling.h"
#include "Abilities/Actions/AlsGameplayAbility_Mantling.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilitySystemComponent)

UAlsAbilitySystemComponent::UAlsAbilitySystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicated(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UAlsAbilitySystemComponent::OnRegister()
{
	Super::OnRegister();
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	if (IsValid(Character))
	{
		Character->OnRefresh.AddUObject(this, &ThisClass::OnRefresh);
		Character->OnContollerChanged.AddUObject(this, &ThisClass::OnControllerChanged);
	}
}

void UAlsAbilitySystemComponent::BindAbilityActivationInput(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action, ETriggerEvent TriggerEvent,
														    const FGameplayTag& InputTag)
{
	auto Handle{EnhancedInputComponent->BindAction(Action, TriggerEvent, this, &ThisClass::ActivateOnInputAction, InputTag).GetHandle()};
	if (!BindingHandles.Contains(InputTag))
	{
		BindingHandles.Add(InputTag);
	}
	BindingHandles[InputTag].AddUnique(Handle);
}

void UAlsAbilitySystemComponent::UnbindAbilityInputs(UEnhancedInputComponent* EnhancedInputComponent, const FGameplayTag& InputTag)
{
	if (BindingHandles.Contains(InputTag))
	{
		for(auto Handle : BindingHandles[InputTag])
		{
			EnhancedInputComponent->RemoveActionBindingForHandle(Handle);
		}
		BindingHandles.Remove(InputTag);
	}
}

void UAlsAbilitySystemComponent::ActivateOnInputAction(FGameplayTag InputTag)
{
	TryActivateAbilitiesBySingleTag(InputTag);
}

void UAlsAbilitySystemComponent::OnControllerChanged_Implementation(AController* PreviousController, AController* NewController)
{
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.IsActive())
		{
			auto* AlsAbility{Cast<UAlsGameplayAbility>(Spec.Ability)};
			if (IsValid(AlsAbility))
			{
				AlsAbility->OnControllerChanged(PreviousController, NewController);
			}
		}
	}
}

void UAlsAbilitySystemComponent::OnRefresh_Implementation(float DeltaTime)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};

	OnTick_Mantling();

	if (Character->GetLocomotionMode() == AlsLocomotionModeTags::InAir && Character->IsLocallyControlled())
	{
		TryActivateAbilitiesBySingleTag(AlsLocomotionActionTags::Mantling);
	}
}

void UAlsAbilitySystemComponent::SetMantlingRootMotion(const FAlsMantlingRootMotionParameters& Parameters)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	if (Character->GetLocalRole() >= ROLE_Authority)
	{
		MulticastSetMantlingRootMotion(Parameters);
	}
	else
	{
		Character->GetCharacterMovement()->FlushServerMoves();

		SetMantlingRootMotionImplementation(Parameters);
		ServerSetMantlingRootMotion(Parameters);
	}
}

void UAlsAbilitySystemComponent::ServerSetMantlingRootMotion_Implementation(const FAlsMantlingRootMotionParameters& Parameters)
{
	MulticastSetMantlingRootMotion(Parameters);
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	Character->ForceNetUpdate();
}

void UAlsAbilitySystemComponent::MulticastSetMantlingRootMotion_Implementation(const FAlsMantlingRootMotionParameters& Parameters)
{
	SetMantlingRootMotionImplementation(Parameters);
}

void UAlsAbilitySystemComponent::SetMantlingRootMotionImplementation(const FAlsMantlingRootMotionParameters& Parameters)
{
	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};
	
	// Reset network smoothing.

	CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;

	Character->GetMesh()->SetRelativeLocationAndRotation(Character->GetBaseTranslationOffset(),
														 Character->GetMesh()->IsUsingAbsoluteRotation()
															? Character->GetActorQuat() * Character->GetBaseRotationOffset()
															: Character->GetBaseRotationOffset(), false, nullptr, ETeleportType::TeleportPhysics);

	// Clear the character movement mode and set the locomotion action to mantling.

	CharacterMovement->SetMovementMode(MOVE_Custom);
	CharacterMovement->SetMovementModeLocked(true);

	const auto* MantlingSettings{Parameters.MantlingSettings.Get()};

	// Calculate actor offsets (offsets between actor and target transform).

	bool bUseRelativeLocation{MovementBaseUtility::UseRelativeLocation(Parameters.TargetPrimitive.Get())};
	const auto TargetRelativeRotation{Parameters.TargetRelativeRotation.GetNormalized()};
	const auto& TargetAnimationLocation{Parameters.TargetAnimationLocation};

	const auto TargetTransform{
		bUseRelativeLocation
			? FTransform{
				TargetRelativeRotation, Parameters.TargetRelativeLocation,
				Parameters.TargetPrimitive->GetComponentScale()
			}.GetRelativeTransformReverse(Parameters.TargetPrimitive->GetComponentTransform())
			: FTransform{TargetRelativeRotation, Parameters.TargetRelativeLocation}
	};

	const auto StartTime{Parameters.StartTime};
	const auto Duration{MantlingSettings->Montage->GetPlayLength() - StartTime};
	const auto PlayRate{MantlingSettings->Montage->RateScale};
	const auto ActorFeetLocationOffset{CharacterMovement->GetActorFeetLocation() - TargetTransform.GetLocation()};
	const auto ActorRotationOffset{TargetTransform.GetRotation().Inverse() * Character->GetActorQuat()};

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

	MantlingRootMotionSourceId = Character->GetCharacterMovement()->ApplyRootMotionSource(RootMotionSource);
}

FAlsRootMotionSource_Mantling* UAlsAbilitySystemComponent::GetCurrentMantlingRootMotionSource() const
{
	if (MantlingRootMotionSourceId == 0)
	{
		return nullptr;
	}

	auto* Character{Cast<AAlsCharacter>(GetOwner())};
	auto* CharacterMovement{Character->GetAlsCharacterMovement()};
	return StaticCastSharedPtr<FAlsRootMotionSource_Mantling>(CharacterMovement->GetRootMotionSourceByID(MantlingRootMotionSourceId)).Get();
}

void UAlsAbilitySystemComponent::OnTick_Mantling()
{
	if (MantlingRootMotionSourceId == 0)
	{
		return;
	}

	if (!HasMatchingGameplayTag(AlsLocomotionActionTags::Mantling))
	{
		auto* RootMotionSource{GetCurrentMantlingRootMotionSource()};

		if (RootMotionSource != nullptr)
		{
			RootMotionSource->Status.SetFlag(ERootMotionSourceStatusFlags::MarkedForRemoval);
		}
		MantlingRootMotionSourceId = 0;

		auto* Character{Cast<AAlsCharacter>(GetOwner())};
		auto* CharacterMovement{Character->GetAlsCharacterMovement()};

		CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

		CharacterMovement->SetMovementModeLocked(false);
		CharacterMovement->SetMovementMode(MOVE_Walking);

		Character->ForceNetUpdate();
	}
}
