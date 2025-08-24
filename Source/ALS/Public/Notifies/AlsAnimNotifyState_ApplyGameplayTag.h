#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AlsGameplayTags.h"
#include "AlsAnimNotifyState_ApplyGameplayTag.generated.h"

UCLASS(DisplayName = "Als Apply GameplayTag Animation Notify State")
class ALS_API UAlsAnimNotifyState_ApplyGameplayTag : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TagToApply;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
