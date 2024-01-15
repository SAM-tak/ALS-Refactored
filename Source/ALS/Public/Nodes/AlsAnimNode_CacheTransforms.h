#pragma once

#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AlsAnimNode_CacheTransforms.generated.h"

class AAlsCharacter;

/**
 *	Debugging node that displays the current value of a bone in a specific space.
 */
USTRUCT(BlueprintInternalUseOnly)
struct ALS_API FAlsAnimNode_CacheTransforms : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	FName CacheName;

	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	TArray<FBoneSocketTarget> TargetNodes;

	AAlsCharacter* AlsCharacter{nullptr};

public:
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;

	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;

	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;

	virtual void GatherDebugData(FNodeDebugData& DebugData) override;

private:
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
};
