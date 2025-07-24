#include "AlsCameraAnimationInstance.h"

#include "AlsCameraRigComponent.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCameraAnimationInstance)

void UAlsCameraAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AAlsCharacter>(GetOwningActor());
	CameraRig = Cast<UAlsCameraRigComponent>(GetSkelMeshComponent());
}

void UAlsCameraAnimationInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!Character.IsValid() || !CameraRig.IsValid())
	{
		return;
	}

	Character->GetOwnedGameplayTags(CurrentGameplayTags);

	CurrentGameplayTags.AddTag(CameraRig->GetConfirmedDesiredViewMode());
	CurrentGameplayTags.AddTag(CameraRig->GetShoulderMode());

	TanHalfVfov = CameraRig->GetTanHalfVfov();
	bFalling = Character->GetLocomotionMode() == AlsLocomotionModeTags::InAir && Character->GetCharacterMovement()->Velocity.Z < -700.f;
}
