#include "Abilities/Tasks/AlsAbilityTask_PlayMontageAndWaitWithNotify.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAbilityTask_PlayMontageAndWaitWithNotify)

UAlsAbilityTask_PlayMontageAndWaitWithNotify::UAlsAbilityTask_PlayMontageAndWaitWithNotify(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UAlsAbilityTask_PlayMontageAndWaitWithNotify* UAlsAbilityTask_PlayMontageAndWaitWithNotify::CreatePlayMontageAndWaitWithNotify(UGameplayAbility* OwningAbility,
	FName TaskInstanceName, UAnimMontage *MontageToPlay, float Rate, FName StartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale, float StartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UAlsAbilityTask_PlayMontageAndWaitWithNotify* MyObj = NewAbilityTask<UAlsAbilityTask_PlayMontageAndWaitWithNotify>(OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->bAllowInterruptAfterBlendOut = bAllowInterruptAfterBlendOut;
	MyObj->StartTimeSeconds = StartTimeSeconds;
	
	return MyObj;
}

void UAlsAbilityTask_PlayMontageAndWaitWithNotify::Activate()
{
	Super::Activate();
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (auto* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
	{
		// AnimInstance's OnPlayMontageNotifyBegin/End fire for all notify. Then stores Montage's InstanceID
		MontageInstanceID = MontageInstance->GetInstanceID();

		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ThisClass::OnNotifyEndReceived);
	}
}

void UAlsAbilityTask_PlayMontageAndWaitWithNotify::OnDestroy(bool AbilityEnded)
{
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ThisClass::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ThisClass::OnNotifyEndReceived);
	}
	Super::OnDestroy(AbilityEnded);
}

bool UAlsAbilityTask_PlayMontageAndWaitWithNotify::IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const
{
	return MontageInstanceID != INDEX_NONE && BranchingPointNotifyPayload.MontageInstanceID == MontageInstanceID;
}

void UAlsAbilityTask_PlayMontageAndWaitWithNotify::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (IsNotifyValid(NotifyName, BranchingPointNotifyPayload))
	{
		OnNotifyBegin.Broadcast(NotifyName);
	}
}

void UAlsAbilityTask_PlayMontageAndWaitWithNotify::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (IsNotifyValid(NotifyName, BranchingPointNotifyPayload))
	{
		OnNotifyEnd.Broadcast(NotifyName);
	}
}
