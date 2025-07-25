#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "Templates/SubclassOf.h"
#include "GameplayTagContainer.h"
#include "AlsAnimNotify_CameraShake.generated.h"

class UCameraShakeBase;

UCLASS(DisplayName = "Als Camera Shake Animation Notify")
class ALSCAMERA_API UAlsAnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float CameraShakeScale{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	ECameraShakePlaySpace PlaySpace{ECameraShakePlaySpace::CameraLocal};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FRotator UserPlaySpaceRot{FRotator::ZeroRotator};

	UPROPERTY(EditAnywhere, Category = Settings, Meta = (FoldProperty))
	FGameplayTagContainer AnyMatchTags;

	UPROPERTY(EditAnywhere, Category = Settings, Meta = (FoldProperty))
	FGameplayTagContainer AllMatchTags;

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;
#if WITH_EDITOR
	virtual void OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent) override;
#endif
};
