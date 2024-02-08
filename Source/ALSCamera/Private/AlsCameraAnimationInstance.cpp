#include "AlsCameraAnimationInstance.h"

#include "AlsCameraSkeletalMeshComponent.h"
#include "AlsCameraComponent.h"
#include "AlsCharacter.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCameraAnimationInstance)

void UAlsCameraAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AAlsCharacter>(GetOwningActor());
	Camera = Cast<UAlsCameraSkeletalMeshComponent>(GetSkelMeshComponent())->GetCameraComponent();
}

void UAlsCameraAnimationInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!Character.IsValid() || !Camera.IsValid())
	{
		return;
	}

	ViewMode = Character->GetViewMode();
	LocomotionMode = Character->GetLocomotionMode();
	RotationMode = Character->GetRotationMode();
	Stance = Character->GetStance();
	Gait = Character->GetGait();
	LocomotionAction = Character->GetLocomotionAction();

	bRightShoulder = Camera->IsRightShoulder();
}
