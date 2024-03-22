#pragma once

#include "AlsLinkedAnimationInstance.h"
#include "AlsCharacterTaskAnimInstance.generated.h"

class UAlsCharacterTask;
struct FAnimUpdateContext;
struct FAnimNodeReference;

UCLASS(Abstract)
class ALS_API UAlsCharacterTaskAnimInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|State", Transient)
	uint8 bCharacterTaskActive : 1{false};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|State", Transient)
	float ObservingFinalBlendWeight{0.0f};

public:
	virtual void Refresh(const UAlsCharacterTask* CharacterTask);

	bool GetCharacterTaskActive()
	{
		return bCharacterTaskActive;
	}

	float GetObservingFinalBlendWeight()
	{
		return ObservingFinalBlendWeight;
	}

protected:
	UFUNCTION(BlueprintCallable, Category = "ALS|Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ObserveBlending(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
};
