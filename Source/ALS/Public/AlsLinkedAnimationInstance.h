#pragma once

#include "Animation/AnimInstance.h"
#include "AlsLinkedAnimationInstance.generated.h"

class AAlsCharacter;

UCLASS(Abstract)
class ALS_API UAlsLinkedAnimationInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|State", Transient)
	TWeakObjectPtr<UAlsAnimationInstance> Parent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|State", Transient)
	TWeakObjectPtr<AAlsCharacter> Character;

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

	TMap<FName, float>& GetAnimationCurvesFromProxy(EAnimCurveType InCurveType);

protected:
	// Be very careful when using this function to read your custom variables using the property access system. It is
	// safe to use this function to read variables that change only inside UAlsAnimationInstance::NativeUpdateAnimation()
	// because it is guaranteed that this function will be called before parallel animation evaluation. Reading variables
	// that change in other functions can be dangerous because they can be changed in the game thread at the same
	// time as being read in the worker thread, which can lead to undefined behavior or even a crash. If you're not
	// sure what you're doing, then it's better to access your custom variables through the "Parent" variable.
	UFUNCTION(BlueprintPure, Category = "ALS|Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	UAlsAnimationInstance* GetParentUnsafe() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void SetHipsDirection(EAlsHipsDirection NewHipsDirection);

	UFUNCTION(BlueprintCallable, Category = "ALS|Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ActivatePivot();

	UFUNCTION(BlueprintCallable, Category = "ALS|Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ResetJumped();
};

inline UAlsAnimationInstance* UAlsLinkedAnimationInstance::GetParentUnsafe() const
{
	return Parent.Get();
}
