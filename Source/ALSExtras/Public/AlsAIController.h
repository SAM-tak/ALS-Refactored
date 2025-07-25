#pragma once

#include "AIController.h"
#include "AlsAIController.generated.h"

UCLASS(DisplayName = "Als AI Controller")
class ALSEXTRAS_API AAlsAIController : public AAIController
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AlsAIController|Settings")
	TObjectPtr<UBehaviorTree> BehaviorTree;

protected:
	virtual void OnPossess(APawn* NewPawn) override;

public:
	virtual FVector GetFocalPointOnActor(const AActor* Actor) const override;
};
