#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "PlayMontageCallbackProxy.h"
#include "AlsAbilityTask_PlayMontageAndWaitWithNotify.generated.h"

/**
 * Task for abilities that supply tick and its' delta time.
 */
UCLASS()
class ALS_API UAlsAbilityTask_PlayMontageAndWaitWithNotify : public UAbilityTask_PlayMontageAndWait
{
    GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnMontagePlayDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable)
	FOnMontagePlayDelegate OnNotifyEnd;

    UFUNCTION(BlueprintCallable, Category = "Als|Ability|Tasks", meta = (DisplayName = "PlayMontageAndWaitWithNotify", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAlsAbilityTask_PlayMontageAndWaitWithNotify* CreatePlayMontageAndWaitWithNotify(UGameplayAbility* OwningAbility,
		FName TaskInstanceName, UAnimMontage* MontageToPlay0, float Rate0 = 1.f, FName StartSection0 = NAME_None, bool bStopWhenAbilityEnds0 = true,
		float AnimRootMotionTranslationScale0 = 1.f, float StartTimeSeconds0 = 0.f, bool bAllowInterruptAfterBlendOut0 = false);

	virtual void Activate() override;

protected:
	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	virtual void OnDestroy(bool AbilityEnded) override;

private:
	int32 MontageInstanceID{INDEX_NONE};

	bool IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const;
};
