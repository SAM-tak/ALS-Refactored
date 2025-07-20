#include "Notifies/AlsAnimNotify_CameraShake.h"

#include "Camera/CameraShakeBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AlsCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAnimNotify_CameraShake)

UAlsAnimNotify_CameraShake::UAlsAnimNotify_CameraShake(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
}

FString UAlsAnimNotify_CameraShake::GetNotifyName_Implementation() const
{
	TStringBuilder<256> NotifyNameBuilder{InPlace, TEXTVIEW("Als Camera Shake: ")};

	if (IsValid(CameraShakeClass))
	{
		NotifyNameBuilder << CameraShakeClass->GetFName();
	}

	return FString{NotifyNameBuilder};
}

void UAlsAnimNotify_CameraShake::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(Mesh, Animation, EventReference);

	if (!AnyMatchTags.IsEmpty() || !AllMatchTags.IsEmpty())
	{
		const auto* Character{Cast<AAlsCharacter>(Mesh->GetOwner())};
		if (!IsValid(Character)
			|| (!AnyMatchTags.IsEmpty() && !Character->HasAnyMatchingGameplayTags(AnyMatchTags))
			|| (!AllMatchTags.IsEmpty() && !Character->HasAllMatchingGameplayTags(AllMatchTags)))
		{
			return;
		}
	}
	
	const auto* Pawn{Cast<APawn>(Mesh->GetOwner())};
	const auto* PlayerController{IsValid(Pawn) ? Cast<APlayerController>(Pawn->GetController()) : nullptr};
	auto* CameraManager{PlayerController && IsValid(PlayerController) ? PlayerController->PlayerCameraManager.Get() : nullptr};

	if (IsValid(CameraManager))
	{
		CameraManager->StartCameraShake(CameraShakeClass, CameraShakeScale, PlaySpace, UserPlaySpaceRot);
	}
}

#if WITH_EDITOR
void UAlsAnimNotify_CameraShake::OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent)
{
	ContainingAnimNotifyEvent.bTriggerOnDedicatedServer = false;
}
#endif
