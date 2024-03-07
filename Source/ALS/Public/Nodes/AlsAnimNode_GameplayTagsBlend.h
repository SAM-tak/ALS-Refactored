#pragma once

#include "GameplayTagContainer.h"
#include "AnimNodes/AnimNode_BlendListBase.h"
#include "AlsAnimNode_GameplayTagsBlend.generated.h"

#if WITH_EDITORONLY_DATA
USTRUCT()
struct ALS_API FAlsGameplayTagContainerMatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere)
	bool bAll{false};
};
#endif

USTRUCT()
struct ALS_API FAlsAnimNode_GameplayTagsBlend : public FAnimNode_BlendListBase
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Settings, Meta = (FoldProperty, PinShownByDefault))
	FGameplayTagContainer Container;

	UPROPERTY(EditAnywhere, Category = Settings, Meta = (FoldProperty))
	TArray<FAlsGameplayTagContainerMatch> TagMatches;
#endif

protected:
	virtual int32 GetActiveChildIndex() override;

public:
	const FGameplayTagContainer& GetContainer() const;

	const TArray<FAlsGameplayTagContainerMatch>& GetTagMatches() const;

#if WITH_EDITOR
	void RefreshPoses();

	static FText GetPosePinName(const FAlsGameplayTagContainerMatch& Match);
#endif
};
