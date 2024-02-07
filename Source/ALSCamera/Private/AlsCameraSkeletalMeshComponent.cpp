#include "AlsCameraSkeletalMeshComponent.h"

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
