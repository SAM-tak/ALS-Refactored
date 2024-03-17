#include "Notifies/AlsAnimNotify_SetGroundedEntryMode.h"

#include "AlsCharacter.h"
#include "AlsAnimationInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Utility/AlsUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAnimNotify_SetGroundedEntryMode)

UAlsAnimNotify_SetGroundedEntryMode::UAlsAnimNotify_SetGroundedEntryMode()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif

	bIsNativeBranchingPoint = true;
}

FString UAlsAnimNotify_SetGroundedEntryMode::GetNotifyName_Implementation() const
{
	return TEXT("Als Set Grounded Entry Mode");
}

void UAlsAnimNotify_SetGroundedEntryMode::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation,
                                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(Mesh, Animation, EventReference);

	auto* Character{Cast<AAlsCharacter>(Mesh->GetOwner())};
	auto* AnimationInstance{Cast<UAlsAnimationInstance>(Mesh->GetAnimInstance())};
	if (IsValid(AnimationInstance))
	{
		for(auto& Value : GroundedEntryMode)
		{
			if (!Value.TagsForMatch.IsValid() || Character->HasAllMatchingGameplayTags(Value.TagsForMatch))
			{
				AnimationInstance->SetGroundedEntryMode(Value.GroundedEntryMode, Value.StartPosition);
				break;
			}
		}
	}
}
