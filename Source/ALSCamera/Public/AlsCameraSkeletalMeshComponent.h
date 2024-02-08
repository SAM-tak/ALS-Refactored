#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "AlsCameraSkeletalMeshComponent.generated.h"

class UAlsCameraComponent;

UCLASS(HideCategories = (Clothing, Physics, MasterPoseComponent, Collision, AnimationRig, Lighting, Deformer,
						 Rendering, PathTracing, HLOD, Navigation, VirtualTexture, SkeletalMesh, Materials,
						 LeaderPoseComponent, Optimization, LOD, MaterialParameters, TextureStreaming, Mobile, RayTracing),
	   ClassGroup = Camera)
class ALSCAMERA_API UAlsCameraSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<UAlsCameraComponent> Camera;

public:
	UAlsCameraSkeletalMeshComponent();

	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void CompleteParallelAnimationEvaluation(const bool bDoPostAnimationEvaluation) override;

	virtual void SetComponentTickEnabled(bool bEnabled) override;

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	void SetCameraComponent(UAlsCameraComponent* NewCameraComponent);

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	UAlsCameraComponent* GetCameraComponent() const;
};

inline UAlsCameraComponent* UAlsCameraSkeletalMeshComponent::GetCameraComponent() const
{
	return Camera.Get();
}
