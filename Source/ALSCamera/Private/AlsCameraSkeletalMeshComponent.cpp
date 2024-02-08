#include "AlsCameraSkeletalMeshComponent.h"
#include "AlsCameraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCameraSkeletalMeshComponent)

UAlsCameraSkeletalMeshComponent::UAlsCameraSkeletalMeshComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bTickInEditor = false;
	bHiddenInGame = true;

	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	SetAllowClothActors(false);
	SetCastShadow(false);
	
#if WITH_EDITOR
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh(TEXT("/ALS/ALSCamera/SKM_Als_Camera"));
	SetSkeletalMeshAsset(Mesh.Object);
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> Animation(TEXT("/ALS/ALSCamera/AB_Als_Camera"));
	SetAnimInstanceClass(Animation.Object->GetAnimBlueprintGeneratedClass());
#endif
}

void UAlsCameraSkeletalMeshComponent::RegisterComponentTickFunctions(const bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	// Tick after the owner to have access to the most up-to-date character state.

	AddTickPrerequisiteActor(GetOwner());
}

void UAlsCameraSkeletalMeshComponent::SetComponentTickEnabled(bool bEnabled)
{
	Super::SetComponentTickEnabled(bEnabled);
	FString Name;
	GetName(Name);
	UE_LOG(LogTemp, Log, TEXT("UAlsCameraSkeletalMeshComponent::SetComponentTickEnabled %s : %s"), bEnabled ? "T" : "F", *Name);
}

void UAlsCameraSkeletalMeshComponent::TickComponent(float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Skip camera tick until parallel animation evaluation completes.

	if (Camera.IsValid() && !IsRunningParallelEvaluation())
	{
		Camera->TickByExtern(DeltaTime);
	}
}

void UAlsCameraSkeletalMeshComponent::CompleteParallelAnimationEvaluation(const bool bDoPostAnimationEvaluation)
{
	Super::CompleteParallelAnimationEvaluation(bDoPostAnimationEvaluation);

	if (Camera.IsValid())
	{
		Camera->TickByExtern(GetAnimInstance()->GetDeltaSeconds());
	}
}

void UAlsCameraSkeletalMeshComponent::SetCameraComponent(UAlsCameraComponent* NewCameraComponent)
{
	Camera = NewCameraComponent;
}
