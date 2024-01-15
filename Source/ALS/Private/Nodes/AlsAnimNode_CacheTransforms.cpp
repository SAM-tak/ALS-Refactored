// Copyright Epic Games, Inc. All Rights Reserved.

#include "Nodes/AlsAnimNode_CacheTransforms.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "AlsCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsAnimNode_CacheTransforms)

/////////////////////////////////////////////////////
// FAnimNode_CacheTransforms

void FAlsAnimNode_CacheTransforms::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)

	TStringBuilder<256> DebugItemBuilder;

	DebugItemBuilder << DebugData.GetNodeName(this) << (AlsCharacter ? "AlsCharacter : OK" : "AlsCharacter : None");

	DebugData.AddDebugItem(FString{DebugItemBuilder});
	ComponentPose.GatherDebugData(DebugData);
}

void FAlsAnimNode_CacheTransforms::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)

	Super::Initialize_AnyThread(Context);

	for (auto& TargetNode : TargetNodes)
	{
		TargetNode.Initialize(Context.AnimInstanceProxy);
	}

	AlsCharacter = Cast<AAlsCharacter>(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
}

void FAlsAnimNode_CacheTransforms::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(EvaluateSkeletalControl_AnyThread)
	ANIM_MT_SCOPE_CYCLE_COUNTER_VERBOSE(CacheTransforms, !IsInGameThread());

	check(OutBoneTransforms.Num() == 0);

	if (AlsCharacter)
	{
		auto& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
		auto& ComponentTransform = Output.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentTransform();
		for (auto& TargetNode : TargetNodes)
		{
			if (TargetNode.HasTargetSetup() && TargetNode.IsValidToEvaluate(BoneContainer))
			{
				AlsCharacter->SetCachedTransform(CacheName,
					TargetNode.bUseSocket ? TargetNode.SocketReference.SocketName : TargetNode.BoneReference.BoneName,
					TargetNode.GetTargetTransform(FTransform::Identity, Output.Pose, ComponentTransform));
			}
		}
	}
}

bool FAlsAnimNode_CacheTransforms::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	if (AlsCharacter)
	{
		for (auto& TargetNode : TargetNodes)
		{
			if (TargetNode.HasTargetSetup() && TargetNode.IsValidToEvaluate(RequiredBones))
			{
				return true;
			}
		}
	}
	return false;
}

void FAlsAnimNode_CacheTransforms::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(InitializeBoneReferences)

	for (auto& TargetNode : TargetNodes)
	{
		TargetNode.InitializeBoneReferences(RequiredBones);
	}
}
