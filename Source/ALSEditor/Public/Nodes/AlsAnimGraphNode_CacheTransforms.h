#pragma once

#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "Nodes/AlsAnimNode_CacheTransforms.h"
#include "AlsAnimGraphNode_CacheTransforms.generated.h"

UCLASS()
class ALSEDITOR_API UAlsAnimGraphNode_CacheTransforms : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAlsAnimNode_CacheTransforms Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;

protected:
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override
	{
		return &Node;
	}

	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
