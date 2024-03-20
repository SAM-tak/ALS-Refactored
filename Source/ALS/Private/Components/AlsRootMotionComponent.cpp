#include "Components/AlsRootMotionComponent.h"

#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "RootMotionSources/AlsRootMotionSource_Mantling.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsLog.h"
#include "Utility/AlsEnumUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsRootMotionComponent)

void UAlsRootMotionComponent::OnRefresh_Implementation(float DeltaTime)
{
	Super::OnRefresh_Implementation(DeltaTime);

	OnTick_Mantling();
}

void UAlsRootMotionComponent::StartMantling(const FAlsMantlingParameters& Parameters)
{
	ensure(Character->GetLocalRole() > ROLE_SimulatedProxy);

	StartMantlingImplementation(Parameters);

	if (Character->HasServerRole())
	{
		MulticastStartMantling(Parameters);
	}
}

void UAlsRootMotionComponent::MulticastStartMantling_Implementation(const FAlsMantlingParameters& Parameters)
{
	if(Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		StartMantlingImplementation(Parameters);
	}
}

void UAlsRootMotionComponent::StartMantlingImplementation(const FAlsMantlingParameters& Parameters)
{
	const auto* Settings{SelectMantlingSettings(Parameters)};

	if (!ALS_ENSURE(IsValid(Settings)) || !ALS_ENSURE(IsValid(Settings->Montage)))
	{
		return;
	}

	const auto StartTime{CalculateMantlingStartTime(Settings, Parameters.MantlingHeight)};
	const auto Montage{Settings->Montage};
	const auto Duration{Montage->GetPlayLength() - StartTime};
	const auto PlayRate{Montage->RateScale};

	const auto TargetAnimationLocation{UAlsUtility::ExtractLastRootTransformFromMontage(Montage).GetLocation()};

	if (FMath::IsNearlyZero(TargetAnimationLocation.Z))
	{
		UE_LOG(LogAls, Warning, TEXT("Can't start mantling! The %s animation montage has incorrect root motion,")
		       TEXT(" the final vertical location of the character must be non-zero!"), *Montage->GetName());
		return;
	}

	// Calculate actor offsets (offsets between actor and target transform).

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

	const auto ActorFeetLocationOffset{Character->GetCharacterMovement()->GetActorFeetLocation() - TargetTransform.GetLocation()};
	const auto ActorRotationOffset{TargetTransform.GetRotation().Inverse() * Character->GetActorQuat()};

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

	// Play the animation montage if valid.

	if (Character->GetMesh()->GetAnimInstance()->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, StartTime, false))
	{
		MantlingMontage = Montage;

		// Apply mantling root motion.

		const auto RootMotionSource{MakeShared<FAlsRootMotionSource_Mantling>()};
		RootMotionSource->InstanceName = __FUNCTION__;
		RootMotionSource->Duration = Duration / PlayRate;
		RootMotionSource->MantlingSettings = Settings;
		RootMotionSource->TargetPrimitive = Parameters.TargetPrimitive;
		RootMotionSource->TargetRelativeLocation = Parameters.TargetRelativeLocation;
		RootMotionSource->TargetRelativeRotation = TargetRelativeRotation;
		RootMotionSource->ActorFeetLocationOffset = ActorFeetLocationOffset;
		RootMotionSource->ActorRotationOffset = ActorRotationOffset.Rotator();
		RootMotionSource->TargetAnimationLocation = TargetAnimationLocation;
		RootMotionSource->MontageStartTime = StartTime;

		MantlingRootMotionSourceId = Character->GetCharacterMovement()->ApplyRootMotionSource(RootMotionSource);
	}
}

float UAlsRootMotionComponent::CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, const float MantlingHeight) const
{
	// https://landelare.github.io/2022/05/15/climbing-with-root-motion.html

	const auto* Montage{MantlingSettings->Montage.Get()};
	if (!IsValid(Montage))
	{
		return 0.0f;
	}

	const auto MontageFrameRate{1.0f / Montage->GetSamplingFrameRate().AsDecimal()};

	auto SearchStartTime{0.0f};
	auto SearchEndTime{Montage->GetPlayLength()};

	const auto SearchStartLocationZ{UAlsUtility::ExtractRootTransformFromMontage(Montage, SearchStartTime).GetTranslation().Z};
	const auto SearchEndLocationZ{UAlsUtility::ExtractRootTransformFromMontage(Montage, SearchEndTime).GetTranslation().Z};

	// Find the vertical distance the character has already moved.

	const auto TargetLocationZ{FMath::Max(0.0f, SearchEndLocationZ - MantlingHeight)};

	// Perform a binary search to find the time when the character is at the target vertical distance.

	static constexpr auto MaxLocationSearchTolerance{1.0f};

	if (FMath::IsNearlyEqual(SearchStartLocationZ, TargetLocationZ, MaxLocationSearchTolerance))
	{
		return SearchStartTime;
	}

	while (true)
	{
		const auto Time{(SearchStartTime + SearchEndTime) * 0.5f};
		const auto LocationZ{UAlsUtility::ExtractRootTransformFromMontage(Montage, Time).GetTranslation().Z};

		// Stop the search if a close enough location has been found or if
		// the search interval is less than the animation montage frame rate.

		if (FMath::IsNearlyEqual(LocationZ, TargetLocationZ, MaxLocationSearchTolerance) ||
		    SearchEndTime - SearchStartTime <= MontageFrameRate)
		{
			return Time;
		}

		if (LocationZ < TargetLocationZ)
		{
			SearchStartTime = Time;
		}
		else
		{
			SearchEndTime = Time;
		}
	}
}

FAlsRootMotionSource_Mantling* UAlsRootMotionComponent::GetCurrentMantlingRootMotionSource() const
{
	if (MantlingRootMotionSourceId == 0)
	{
		return nullptr;
	}

	auto* CharacterMovement{Character->GetAlsCharacterMovement()};
	return StaticCastSharedPtr<FAlsRootMotionSource_Mantling>(CharacterMovement->GetRootMotionSourceByID(MantlingRootMotionSourceId)).Get();
}

void UAlsRootMotionComponent::OnTick_Mantling()
{
	if (MantlingRootMotionSourceId == 0)
	{
		return;
	}

	if (!Character->HasMatchingGameplayTag(AlsLocomotionActionTags::Mantling))
	{
		auto* RootMotionSource{GetCurrentMantlingRootMotionSource()};

		if (RootMotionSource != nullptr)
		{
			RootMotionSource->Status.SetFlag(ERootMotionSourceStatusFlags::MarkedForRemoval);
		}
		MantlingRootMotionSourceId = 0;
		MantlingMontage.Reset();

		auto* CharacterMovement{Character->GetAlsCharacterMovement()};

		CharacterMovement->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

		CharacterMovement->SetMovementModeLocked(false);
		CharacterMovement->SetMovementMode(MOVE_Walking);

		Character->ForceNetUpdate();
	}
}

UAlsMantlingSettings* UAlsRootMotionComponent::SelectMantlingSettings_Implementation(const FAlsMantlingParameters& Parameters) const
{
	return nullptr;
}
