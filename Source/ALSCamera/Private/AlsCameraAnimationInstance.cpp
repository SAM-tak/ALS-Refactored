#include "AlsCameraAnimationInstance.h"

#include "AlsCameraMovementComponent.h"
#include "AlsCharacter.h"
#include "AlsCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCameraAnimationInstance)

void UAlsCameraAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AAlsCharacter>(GetOwningActor());
	CameraMovement = Cast<UAlsCameraMovementComponent>(GetSkelMeshComponent());
}

void UAlsCameraAnimationInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!Character.IsValid() || !CameraMovement.IsValid())
	{
		return;
	}

	Character->GetOwnedGameplayTags(CurrentGameplayTags);

	CurrentGameplayTags.AddTag(CameraMovement->GetConfirmedDesiredViewMode());
	CurrentGameplayTags.AddTag(CameraMovement->GetShoulderMode());

	TanHalfVfov = CameraMovement->GetTanHalfVfov();
	bFalling = Character->GetLocomotionMode() == AlsLocomotionModeTags::InAir && Character->GetCharacterMovement()->Velocity.Z < -700.f;
}
