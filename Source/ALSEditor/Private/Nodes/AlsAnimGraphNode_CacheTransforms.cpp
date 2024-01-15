#include "Nodes/AlsAnimGraphNode_CacheTransforms.h"
#include "AnimationGraphSchema.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "AlsCacheTransformsAnimationGraphNode"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAnimGraphNode_CacheTransforms)

FText UAlsAnimGraphNode_CacheTransforms::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	if (((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle)) && (Node.CacheName == NAME_None))
	{
		return LOCTEXT("AnimGraphNode_ObserveBone_Title", "Cache Transforms");
	}
	else
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("CacheName"), FText::FromName(Node.CacheName));

		return FText::Format(LOCTEXT("AnimGraphNode_ObserveBone_Title", "Cache Transforms: {CacheName}"), Args);
	}
}

FText UAlsAnimGraphNode_CacheTransforms::GetTooltipText() const
{
	return LOCTEXT("UAlsAnimGraphNode_CacheTransforms_Tooltip", "Save Intermidiate Bone Transforms For ADS (etc)");
}

FString UAlsAnimGraphNode_CacheTransforms::GetNodeCategory() const
{
	return FString{TEXTVIEW("ALS")};
}

void UAlsAnimGraphNode_CacheTransforms::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const
{
	if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, Alpha) ||
		Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, AlphaCurveName))
	{
		Pin->bHidden = true;
	}
}

void UAlsAnimGraphNode_CacheTransforms::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	Super::CustomizeDetails(DetailBuilder);

	auto NodeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAlsAnimGraphNode_CacheTransforms, Node), GetClass());
	DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, Alpha)));
	DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, AlphaScaleBias)));
	DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_SkeletalControlBase, AlphaCurveName)));
}

#undef LOCTEXT_NAMESPACE
