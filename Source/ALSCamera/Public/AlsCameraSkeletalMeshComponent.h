#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "AlsCameraSkeletalMeshComponent.generated.h"

UCLASS(HideCategories = (ComponentTick, Clothing, Physics, MasterPoseComponent, Collision, AnimationRig,
	Lighting, Deformer, Rendering, PathTracing, HLOD, Navigation, VirtualTexture, SkeletalMesh, Materials,
	LeaderPoseComponent, Optimization, LOD, MaterialParameters, TextureStreaming, Mobile, RayTracing),
	ClassGroup = Camera)
class ALSCAMERA_API UAlsCameraSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UAlsCameraSkeletalMeshComponent();

	virtual void RegisterComponentTickFunctions(bool bRegister) override;
};
