#pragma once

#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AlsAnimNotify_SetGroundedEntryMode.generated.h"

USTRUCT(BlueprintType)
struct FGroundedEntryMode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FGameplayTag GroundedEntryMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float StartPosition{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FGameplayTagContainer TagsForMatch;
};

UCLASS(DisplayName = "Als Set Grounded Entry Mode Animation Notify")
class ALS_API UAlsAnimNotify_SetGroundedEntryMode : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TArray<FGroundedEntryMode> GroundedEntryMode;

public:
	UAlsAnimNotify_SetGroundedEntryMode();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;
};
