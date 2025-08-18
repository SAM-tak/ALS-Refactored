#include "AlsGameplayCameraStateComponent.h"

#include "AlsGameplayCameraStateSettings.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/GameplayCameraComponentBase.h"
#include "Core/CameraSystemEvaluator.h"
#include "Core/CameraVariableAssets.h"
#include "Core/RootCameraNode.h"
#include "Curves/CurveFloat.h"
#include "Engine/OverlapResult.h"
#include "Misc/UObjectToken.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Utility/AlsCameraConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsGameplayCameraStateComponent)

UAlsGameplayCameraStateComponent::UAlsGameplayCameraStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	bAutoActivate = true;
	bTickInEditor = false;

	SetIsReplicatedByDefault(true);
	SetCanEverAffectNavigation(false);
}

void UAlsGameplayCameraStateComponent::OnRegister()
{
	Super::OnRegister();

	Character = GetPawn<AAlsCharacter>();

	if (!Character.IsValid())
	{
		UE_LOG(LogAls, Error, TEXT("[UAlsCharacterComponent::OnRegister] This component has been added to a blueprint whose base class is not a child of AlsCharacter. To use this component, it MUST be placed on a child of AlsCharacter Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("AlsCharacterComponent", "NotOnAlsCharacterError", "has been added to a blueprint whose base class is not a child of AlsCharacter. To use this component, it MUST be placed on a child of AlsCharacter Blueprint. This will cause a crash if you PIE!");
			static const FName MessageLogName = TEXT("AlsCharacterComponent");
			
			FMessageLog(MessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
			
			FMessageLog(MessageLogName).Open();
		}
#endif
	}

	GameplayCameraComponent = Character->GetComponentByClass<UGameplayCameraComponentBase>();
}

void UAlsGameplayCameraStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ConfirmedDesiredViewMode, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ShoulderMode, Parameters)
}

void UAlsGameplayCameraStateComponent::Activate(const bool bReset)
{
	Super::Activate(bReset);
#if !UE_BUILD_SHIPPING
	if (Character.IsValid())
	{
		Character->OnDisplayDebug.AddUObject(this, &ThisClass::DisplayDebug);
	}
#endif
	SetComponentTickEnabled(true);

	if (!bReset && !ShouldActivate())
	{
		return;
	}
}

void UAlsGameplayCameraStateComponent::Deactivate()
{
	SetComponentTickEnabled(false);
#if !UE_BUILD_SHIPPING
	if (Character.IsValid())
	{
		Character->OnDisplayDebug.RemoveAll(this);
	}
#endif
	Super::Deactivate();
}

void UAlsGameplayCameraStateComponent::InitializeByCameraVariables(
	UVector3dCameraVariable* BoomOffsetVariable,
	UVector3dCameraVariable* CenterShoulderOffsetVariable,
	UVector3dCameraVariable* LeftShoulderOffsetVariable,
	UVector3dCameraVariable* RightShoulderOffsetVariable)
{
	if (IsValid(BoomOffsetVariable))
	{
		BoomOffset = BoomOffsetVariable->GetDefaultValue();
	}
	if (IsValid(CenterShoulderOffsetVariable))
	{
		CenterShoulderOffset = CenterShoulderOffsetVariable->GetDefaultValue();
	}
	if (IsValid(LeftShoulderOffsetVariable))
	{
		LeftShoulderOffset = LeftShoulderOffsetVariable->GetDefaultValue();
	}
	if (IsValid(RightShoulderOffsetVariable))
	{
		RightShoulderOffset = RightShoulderOffsetVariable->GetDefaultValue();
	}
}

void UAlsGameplayCameraStateComponent::BindCameraVariables(UFloatCameraVariable* FirstPersonFactorVariable,
	UFloatCameraVariable* TraceSphreRadiusVariable,
	UVector3dCameraVariable* FirstPersonLocationVariable,
	UVector3dCameraVariable* EyeLocationVariable,
	UVector3dCameraVariable* ADSLocationVariable,
	URotator3dCameraVariable* ADSRotationVariable)
{
	if (FirstPersonFactorVariable)
	{
		FirstPersonFactorVariableId = FirstPersonFactorVariable->GetVariableID();
	}
	if (TraceSphreRadiusVariable)
	{
		TraceSphreRadiusVariableId = TraceSphreRadiusVariable->GetVariableID();
	}

	auto Context = GameplayCameraComponent->GetEvaluationContext();
	if (!Context.IsValid())
	{
		return;
	}
	auto& VariableTable = Context->GetInitialResult().VariableTable;
	if (FirstPersonLocationVariable)
	{
		FirstPersonLocationVariableId = FirstPersonLocationVariable->GetVariableID();
		if (!VariableTable.ContainsValue(FirstPersonLocationVariableId))
		{
			VariableTable.AddVariable(FirstPersonLocationVariable->GetVariableDefinition());
			UE_LOG(LogAls, Log, TEXT("Added FirstPersonLocation variable to VariableTable: %s"), *FirstPersonLocationVariable->GetDisplayName());
		}
	}
	if (EyeLocationVariable)
	{
		EyeLocationVariableId = EyeLocationVariable->GetVariableID();
		if (!VariableTable.ContainsValue(EyeLocationVariableId))
		{
			VariableTable.AddVariable(EyeLocationVariable->GetVariableDefinition());
			UE_LOG(LogAls, Log, TEXT("Added EyeLocation variable to VariableTable: %s"), *EyeLocationVariable->GetDisplayName());
		}
	}
	if (ADSLocationVariable)
	{
		ADSLocationVariableId = ADSLocationVariable->GetVariableID();
		if (!VariableTable.ContainsValue(ADSLocationVariableId))
		{
			VariableTable.AddVariable(ADSLocationVariable->GetVariableDefinition());
			UE_LOG(LogAls, Log, TEXT("Added ADSLocation variable to VariableTable: %s"), *ADSLocationVariable->GetDisplayName());
		}
	}
	if (ADSRotationVariable)
	{
		ADSRotationVariableId = ADSRotationVariable->GetVariableID();
		if (!VariableTable.ContainsValue(ADSRotationVariableId))
		{
			VariableTable.AddVariable(ADSRotationVariable->GetVariableDefinition());
			UE_LOG(LogAls, Log, TEXT("Added ADSRotation variable to VariableTable: %s"), *ADSRotationVariable->GetDisplayName());
		}
	}
}

void UAlsGameplayCameraStateComponent::BeginPlay()
{
	Super::BeginPlay();

	PreviousShoulderMode = ShoulderMode = Settings->ThirdPerson.ShoulderMode;
	PreviousConfirmedDesiredViewMode = DesiredViewMode = Settings->DesiredViewMode;
	SetConfirmedDesiredViewMode(DesiredViewMode);
	Character->SetViewMode(ViewMode == AlsCameraViewModeTags::ThirdPerson ? AlsViewModeTags::ThirdPerson : AlsViewModeTags::FirstPerson);
}

void UAlsGameplayCameraStateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UAlsGameplayCameraStateComponent::TickComponent()"), STAT_UAlsGameplayCameraStateComponent_Tick, STATGROUP_Als)

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(Settings) || !Character.IsValid() || !GameplayCameraComponent.IsValid())
	{
		return;
	}

	FirstPersonFactor = 0.0f;

	auto CameraSystemEvaluator = GameplayCameraComponent->GetCameraSystemEvaluator();
	if (CameraSystemEvaluator.IsValid())
	{
		const auto& Result = CameraSystemEvaluator->GetEvaluatedResult();
		CameraLocation = Result.CameraPose.GetLocation();
		CameraRotation = Result.CameraPose.GetRotation();

		auto* PlayerController{Cast<APlayerController>(Character->GetController())};
		if (GameplayCameraComponent.IsValid() && IsValid(PlayerController))
		{
			TanHalfVfov = FMath::Tan(FMath::DegreesToRadians(Result.CameraPose.GetEffectiveFieldOfView()) * 0.5f);
			if (Result.CameraPose.GetPanoramic())
			{
				int32 SizeX, SizeY;
				PlayerController->GetViewportSize(SizeX, SizeY);
				float AspectRatio{SizeX * (1 - Result.CameraPose.GetPanoramaSideViewRate() * 2 / 3) / (float)SizeY};
				TanHalfVfov /= AspectRatio;
			}
		}

		if(FirstPersonFactorVariableId.IsValid())
		{
			float OutValue;
			if (Result.VariableTable.TryGetValue<float>(FirstPersonFactorVariableId, OutValue))
			{
				FirstPersonFactor = UAlsMath::Clamp01(OutValue);
			}
		}
		if(TraceSphreRadiusVariableId.IsValid())
		{
			float OutValue;
			if (Result.VariableTable.TryGetValue<float>(TraceSphreRadiusVariableId, OutValue))
			{
				TraceSphreRadius = FMath::Max(OutValue, 0.001f);
			}
		}
	}

#if ENABLE_DRAW_DEBUG
	const auto bDisplayDebugCameraShapes{
		UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraShapesDebugDisplayName())
	};
	const auto bDisplayDebugCameraTraces{
		UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraTracesDebugDisplayName())
	};
#endif

	// Refresh desired view mode information.

	if (Character->IsCharacterSelf())
	{
		if (ViewModeChangeBlockTime > 0.f)
		{
			ViewModeChangeBlockTime -= DeltaTime;
		}
		else
		{
			if (DesiredViewMode != ConfirmedDesiredViewMode)
			{
				ViewModeChangeBlockTime = Settings->ViewModeChangeBlockTime;
			}
			SetConfirmedDesiredViewMode(DesiredViewMode);
		}
	}

	if (PreviousConfirmedDesiredViewMode != ConfirmedDesiredViewMode)
	{
		// Set aim point correction during change FPP/TPP
		auto FocusLocation{GetCurrentFocusLocation()};
		if (PreviousConfirmedDesiredViewMode == AlsCameraViewModeTags::FirstPerson)
		{
			// FPP -> TPP
			auto TraceStart{GetThirdPersonTraceStartLocation()};
			auto FocalRotation{(FocusLocation - TraceStart).Rotation()};
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			if (Settings->HeuristicPitchMapping && IsValid(Settings->HeuristicPitchMapping))
			{
				FocalRotation.Normalize();
				auto Mapped = FMath::Lerp(-180.0f, 180.0f, Settings->HeuristicPitchMapping->GetFloatValue((FocalRotation.Pitch + 180.0f) / 360.0f));
				//UE_LOG(LogTemp, Log, TEXT("%.2f -> %.2f"), FocalRotation.Pitch, Mapped);
				FocalRotation.Pitch = Mapped;
			}
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), TraceStart, FocusLocation, FLinearColor{0.0f, 0.75f, 1.0f}.ToFColor(true),
					false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		else if(PreviousConfirmedDesiredViewMode == AlsCameraViewModeTags::ThirdPerson)
		{
			// TPP -> FPP
			auto TraceStart{GetFirstPersonTraceStartLocation()};
			auto FocalRotation{(FocusLocation - TraceStart).Rotation()};
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			if (Settings->HeuristicPitchMapping && IsValid(Settings->HeuristicPitchMapping))
			{
				FocalRotation.Normalize();
				auto Mapped = FMath::Lerp(-180.0f, 180.0f, Settings->HeuristicPitchMapping->GetFloatValue((FocalRotation.Pitch + 180.0f) / 360.0f));
				//UE_LOG(LogTemp, Log, TEXT("%.2f -> %.2f"), FocalRotation.Pitch, Mapped);
				FocalRotation.Pitch = Mapped;
			}
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), TraceStart, FocusLocation, FLinearColor{0.75f, 0.0f, 1.0f}.ToFColor(true),
					false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		PreviousConfirmedDesiredViewMode = ConfirmedDesiredViewMode;
	}

	if (PreviousShoulderMode != ShoulderMode)
	{
		if (Character->GetViewMode() == AlsViewModeTags::ThirdPerson && bIsFocusPawn)
		{
			// Set aim point correction during change shoulder
			auto FocusLocation{GetCurrentFocusLocation()};
			auto NextCameraLocation{GetThirdPersonTraceStartLocation()};
			auto FocalRotation{(FocusLocation - NextCameraLocation).Rotation()};
			FocalRotation.Roll = Character->GetViewRotation().Roll;
			Character->SetFocalRotation(FocalRotation);
#if ENABLE_DRAW_DEBUG
			if (bDisplayDebugCameraTraces)
			{
				DrawDebugLine(GetWorld(), NextCameraLocation, FocusLocation, FLinearColor{0.0f, 0.75f, 1.0f}.ToFColor(true),
					false, 3.0f, 0, UAlsUtility::DrawLineThickness);
			}
#endif
		}
		PreviousShoulderMode = ShoulderMode;
	}

	UpdateViewMode();
	UpdateFocalLength();

	if (FirstPersonFactor > Settings->FirstPerson.FirstPersonFactorThreshold)
	{
		Character->SetLookRotation(Character->GetViewRotation());
	}
	else
	{
		if (Character->HasMatchingGameplayTag(AlsAimingModeTags::AimDownSight))
		{
			if (bIsSightOffsetValid)
			{
				auto ControlRotation = Character->GetControlRotation();
				auto Location{ControlRotation.RotateVector(SightLocationOffset) + GetEyeCameraLocation()};
				auto Rotation{(ControlRotation.Quaternion() * SightRotationOffset).Rotator()};
				Location = FVector::PointPlaneProject(Location, GetEyeCameraLocation(), Rotation.Vector())
					- Rotation.Vector() * Settings->FirstPerson.RetreatDistance;
				Character->SetLookRotation((GetCurrentFocusLocation() - Location).Rotation());
			}
			else
			{
				Character->SetLookRotation((GetCurrentFocusLocation() - GetEyeCameraLocation()).Rotation());
			}
		}
		else
		{
			Character->SetLookRotation((GetCurrentFocusLocation() - GetFirstPersonCameraLocation()).Rotation());
		}
	}

	Character->SetViewMode(FirstPersonFactor > Settings->FirstPerson.FirstPersonFactorThreshold ? AlsViewModeTags::FirstPerson : AlsViewModeTags::ThirdPerson);
}

void UAlsGameplayCameraStateComponent::UpdateState(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UAlsGameplayCameraStateComponent::Update()"), STAT_UAlsGameplayCameraStateComponent_Update, STATGROUP_Als)

	if (!IsValid(Settings) || !Character.IsValid() || !GameplayCameraComponent.IsValid())
	{
		return;
	}

	GameplayCameraComponent->SetRelativeLocation(FVector{0.0f, 0.0f, Character->BaseEyeHeight});

	auto Context = GameplayCameraComponent->GetEvaluationContext();
	if (!Context.IsValid())
	{
		return;
	}

	auto& VariableTable = Context->GetInitialResult().VariableTable;
	auto ControlRotation = Character->GetControlRotation();
	if (FirstPersonLocationVariableId.IsValid())
	{
		VariableTable.SetValue<FVector3d>(FirstPersonLocationVariableId, GetFirstPersonCameraLocation()
			- ControlRotation.Vector() * Settings->FirstPerson.RetreatDistance);
	}
	if (EyeLocationVariableId.IsValid())
	{
		VariableTable.SetValue<FVector3d>(EyeLocationVariableId, GetEyeCameraLocation()
			- ControlRotation.Vector() * Settings->FirstPerson.RetreatDistance);
	}

	if(Character->GetAimAmount() < Settings->FirstPerson.ADSThreshold)
	{
		bIsSightOffsetValid = false;
	}

	FVector Location;
	FRotator Rotation;
	if(Character->HasSight() && Character->GetAimAmount() >= Settings->FirstPerson.ADSThreshold
		&& Character->HasMatchingGameplayTag(AlsAimingModeTags::AimDownSight)
		&& !Character->HasAnyMatchingGameplayTags(Settings->FirstPerson.RecoilStateTags))
	{
		Character->GetSightLocAndRot(Location, Rotation);
		Rotation.Roll = Character->GetControlRotation().Roll;
		auto ControlRotationInverse{ControlRotation.Quaternion().Inverse()};
		SightLocationOffset = ControlRotationInverse.RotateVector(Location - GetEyeCameraLocation());
		SightRotationOffset = Rotation.Quaternion() * ControlRotationInverse;
		bIsSightOffsetValid = true;
		Location = FVector::PointPlaneProject(Location, GetEyeCameraLocation(), Rotation.Vector())
			- Rotation.Vector() * Settings->FirstPerson.RetreatDistance;
	}
	else if(bIsSightOffsetValid)
	{
		Location = ControlRotation.RotateVector(SightLocationOffset) + GetEyeCameraLocation();
		Rotation = (ControlRotation.Quaternion() * SightRotationOffset).Rotator();
		Rotation.Roll = Character->GetControlRotation().Roll;
		Location = FVector::PointPlaneProject(Location, GetEyeCameraLocation(), Rotation.Vector())
			- Rotation.Vector() * Settings->FirstPerson.RetreatDistance;
	}
	else
	{
		Location = GetEyeCameraLocation() - ControlRotation.Vector() * Settings->FirstPerson.RetreatDistance;
		Rotation = ControlRotation;
	}

	if (ADSLocationVariableId.IsValid())
	{
		VariableTable.SetValue<FVector3d>(ADSLocationVariableId, Location);
	}
	if (ADSRotationVariableId.IsValid())
	{
		VariableTable.SetValue<FRotator3d>(ADSRotationVariableId, Rotation);
	}
}

FVector UAlsGameplayCameraStateComponent::GetFirstPersonCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(Settings->FirstPerson.CameraSocketName);
}

FVector UAlsGameplayCameraStateComponent::GetEyeCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(Settings->FirstPerson.bLeftDominantEye
												   ? Settings->FirstPerson.LeftEyeCameraSocketName
												   : Settings->FirstPerson.RightEyeCameraSocketName);
}

FVector UAlsGameplayCameraStateComponent::GetThirdPersonTraceStartLocation() const
{
	auto ShoulderOffset{
		ShoulderMode == AlsCameraShoulderModeTags::Right ? RightShoulderOffset :
		ShoulderMode == AlsCameraShoulderModeTags::Left ? LeftShoulderOffset :
		CenterShoulderOffset
	};
	return GameplayCameraComponent->GetComponentLocation() + CameraRotation.RotateVector(BoomOffset) + CameraRotation.RotateVector(ShoulderOffset);
}

FVector UAlsGameplayCameraStateComponent::GetFirstPersonTraceStartLocation() const
{
	auto ViewRotation = Character->GetViewRotation();
	if (Character->HasMatchingGameplayTag(AlsAimingModeTags::AimDownSight))
	{
		if (bIsSightOffsetValid)
		{
			auto Location{ViewRotation.RotateVector(SightLocationOffset) + GetEyeCameraLocation()};
			auto Rotation{(ViewRotation.Quaternion() * SightRotationOffset).Rotator()};
			return FVector::PointPlaneProject(Location, GetEyeCameraLocation(), Rotation.Vector())
				- Rotation.Vector() * Settings->FirstPerson.RetreatDistance;
		}
		else
		{
			return GetEyeCameraLocation() - ViewRotation.Vector() * Settings->FirstPerson.RetreatDistance;
		}
	};
	return GetFirstPersonCameraLocation() - ViewRotation.Vector() * Settings->FirstPerson.RetreatDistance;
}

void UAlsGameplayCameraStateComponent::UpdateViewMode()
{
	if (ConfirmedDesiredViewMode == AlsCameraViewModeTags::FirstPerson)
	{
		ViewMode = AlsCameraViewModeTags::FirstPerson;
		return;
	}
	if (Settings->ThirdPerson.AutoFPPStartDistance <= 0.0f || Character->GetLocomotionAction().IsValid())
	{
		ViewMode = AlsCameraViewModeTags::ThirdPerson;
		return;
	}

	// Auto FPP processing

	static const FName MainTraceTag{FString::Printf(TEXT("%hs (Main Trace)"), __FUNCTION__)};
	auto TraceStart{GetThirdPersonTraceStartLocation()};
	auto TraceEnd{GetThirdPersonTraceStartLocation() - CameraRotation.Vector() * Settings->ThirdPerson.AutoFPPEndDistance};
	const auto CollisionShape{FCollisionShape::MakeSphere(TraceSphreRadius)};
	auto TraceResult{TraceEnd};
	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->ThirdPerson.TraceChannel, CollisionShape,
		{MainTraceTag, false, Character.Get()}))
	{
		if (!Hit.bStartPenetrating)
		{
			TraceResult = Hit.Location;
		}
		else
		{
			TraceResult = TraceStart;
		}
	}
#if ENABLE_DRAW_DEBUG
	if (UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraTracesDebugDisplayName()))
	{
		UAlsUtility::DrawDebugSweepSphere(GetWorld(), TraceStart, TraceResult, CollisionShape.GetCapsuleRadius(),
			Hit.IsValidBlockingHit() ? FLinearColor::Red : FLinearColor::Green);
	}
#endif
	auto Distance{FVector::Dist(TraceStart, TraceResult)};
	if (ViewMode == AlsCameraViewModeTags::FirstPerson && Distance > Settings->ThirdPerson.AutoFPPEndDistance)
	{
		ViewMode = AlsCameraViewModeTags::ThirdPerson;
	}
	else if(ViewMode == AlsCameraViewModeTags::ThirdPerson && Distance < Settings->ThirdPerson.AutoFPPStartDistance)
	{
		ViewMode = AlsCameraViewModeTags::FirstPerson;
	}
}

void UAlsGameplayCameraStateComponent::UpdateFocalLength()
{
	static const FName MainTraceTag{FString::Printf(TEXT("%hs (Main Trace)"), __FUNCTION__)};
	const auto CollisionShape{FCollisionShape::MakeSphere(Settings->FocusTraceRadius)};

	auto EyeVec = CameraRotation.Vector();
	FVector TraceStart = CameraLocation + EyeVec * Settings->MinFocalLength;
	if (Character->GetViewMode() != AlsViewModeTags::FirstPerson)
	{
		auto ProjectedLocation = FVector::PointPlaneProject(CameraLocation, Character->GetActorLocation(), -EyeVec);
		auto ProjectedLocationDistance = FVector::Distance(TraceStart, ProjectedLocation);
		TraceStart = ProjectedLocation + EyeVec * FMath::Max(-ProjectedLocationDistance, Settings->ThirdPerson.FocusTraceStartOffset);
	}
	FVector TraceEnd{CameraLocation + EyeVec * Settings->MaxFocalLength};
	FVector TraceResult{TraceEnd};

	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, Settings->FocusTraceChannel,
										 CollisionShape, {MainTraceTag, false, Character.Get()}))
	{
		TraceResult = Hit.Location;
		if (Hit.HasValidHitObjectHandle())
		{
			bIsFocusPawn = IsValid(Cast<APawn>(Hit.GetActor()));
		}
	}
	else
	{
		bIsFocusPawn = false;
	}

#if ENABLE_DRAW_DEBUG
	if (UAlsUtility::ShouldDisplayDebugForActor(Character.Get(), UAlsCameraConstants::CameraTracesDebugDisplayName()))
	{
		UAlsUtility::DrawDebugSweepSphere(GetWorld(), TraceStart, TraceResult, CollisionShape.GetSphereRadius(),
			Hit.IsValidBlockingHit() ? FLinearColor::Red : FLinearColor::Green);
	}
#endif

	FocalLength = FMath::Max(Settings->MinFocalLength, FVector::Distance(TraceResult, CameraLocation));
}

void UAlsGameplayCameraStateComponent::SetDesiredViewMode(const FGameplayTag& NewDesiredViewMode)
{
	DesiredViewMode = NewDesiredViewMode;
}

void UAlsGameplayCameraStateComponent::SetConfirmedDesiredViewMode(const FGameplayTag& NewConfirmedDesiredViewMode)
{
	if (ConfirmedDesiredViewMode == NewConfirmedDesiredViewMode || Character->GetLocalRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	ConfirmedDesiredViewMode = NewConfirmedDesiredViewMode;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ConfirmedDesiredViewMode, this)

	if (Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSetConfirmedDesiredViewMode(NewConfirmedDesiredViewMode);
	}
}

void UAlsGameplayCameraStateComponent::ServerSetConfirmedDesiredViewMode_Implementation(const FGameplayTag& NewViewMode)
{
	SetConfirmedDesiredViewMode(NewViewMode);
}

void UAlsGameplayCameraStateComponent::SetShoulderMode(const FGameplayTag& NewShoulderMode)
{
	if (ShoulderMode == NewShoulderMode || Character->GetLocalRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	ShoulderMode = NewShoulderMode;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ShoulderMode, this)

	if (Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSetShoulderMode(NewShoulderMode);
	}
}

void UAlsGameplayCameraStateComponent::ServerSetShoulderMode_Implementation(const FGameplayTag& NewShoulderMode)
{
	SetShoulderMode(NewShoulderMode);
}

void UAlsGameplayCameraStateComponent::ToggleShoulder()
{
	if (ShoulderMode != AlsCameraShoulderModeTags::Center)
	{
		SetShoulderMode(ShoulderMode == AlsCameraShoulderModeTags::Right ? AlsCameraShoulderModeTags::Left : AlsCameraShoulderModeTags::Right);
	}
}
