#include "Abilities/Tasks/AlsAbilityTask_PlayLocalMontage.h"
#include "AlsCharacter.h"
#include "Components/AlsLocalMontageComponent.h"
#include "CharacterTasks/AlsLocalMontageTask.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilityTask_PlayLocalMontage)

UAlsAbilityTask_PlayLocalMontage::UAlsAbilityTask_PlayLocalMontage(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UAlsAbilityTask_PlayLocalMontage* UAlsAbilityTask_PlayLocalMontage::CreatePlayLocalMontage(UGameplayAbility* OwningAbility, FName TaskInstanceName,
																						   FGameplayTag _LocalMontageTag)
{
	UAlsAbilityTask_PlayLocalMontage* MyObj = NewAbilityTask<UAlsAbilityTask_PlayLocalMontage>(OwningAbility, TaskInstanceName);
	MyObj->LocalMontageTag = _LocalMontageTag;
	return MyObj;
}

void UAlsAbilityTask_PlayLocalMontage::Activate()
{
	Super::Activate();
	const auto* ActorInfo = Ability->GetCurrentActorInfo();
	auto* Character{Cast<AAlsCharacter>(ActorInfo->OwnerActor)};
	auto* Component{Character->FindComponentByClass<UAlsLocalMontageComponent>()};

	LocalMontageTask = Component->ChangeLocalMontageTaskIfNeeded(LocalMontageTag);
	if (LocalMontageTask.IsValid())
	{
		LocalMontageTask->CurrentAbilityTask = this;
	}
}

void UAlsAbilityTask_PlayLocalMontage::OnDestroy(bool AbilityEnded)
{
	if (LocalMontageTask.IsValid())
	{
		LocalMontageTask->End();
		LocalMontageTask->CurrentAbilityTask.Reset();
	}

	Super::OnDestroy(AbilityEnded);
}
