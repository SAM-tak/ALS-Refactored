#include "AlsLinkedAnimationInstance.h"

#include "AlsAnimationInstance.h"
#include "AlsAnimationInstanceProxy.h"
#include "AlsCharacter.h"
#include "Utility/AlsMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsLinkedAnimationInstance)

UAlsLinkedAnimationInstance::UAlsLinkedAnimationInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bUseMainInstanceMontageEvaluationData = true;
}

void UAlsLinkedAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Parent = Cast<UAlsAnimationInstance>(GetSkelMeshComponent()->GetAnimInstance());
	Character = Cast<AAlsCharacter>(GetOwningActor());

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// Use default objects for editor preview.

		if (!Parent.IsValid())
		{
			Parent = GetMutableDefault<UAlsAnimationInstance>();
		}

		if (!Character.IsValid())
		{
			Character = GetMutableDefault<AAlsCharacter>();
		}
	}
#endif
}

void UAlsLinkedAnimationInstance::NativeBeginPlay()
{
	ALS_ENSURE_MESSAGE(Parent.IsValid(),
	                   TEXT("%s (%s) should only be used as a linked animation instance within the %s animation blueprint!"),
	                   ALS_GET_TYPE_STRING(UAlsLinkedAnimationInstance).GetData(), *GetClass()->GetName(),
	                   ALS_GET_TYPE_STRING(UAlsAnimationInstance).GetData());

	Super::NativeBeginPlay();
}

FAnimInstanceProxy* UAlsLinkedAnimationInstance::CreateAnimInstanceProxy()
{
	return new FAlsAnimationInstanceProxy{this};
}

TMap<FName, float>& UAlsLinkedAnimationInstance::GetAnimationCurvesFromProxy(EAnimCurveType InCurveType)
{
	return GetProxyOnAnyThread<FAlsAnimationInstanceProxy>().GetAnimationCurves(EAnimCurveType::AttributeCurve);
}

void UAlsLinkedAnimationInstance::SetHipsDirection(const EAlsHipsDirection NewHipsDirection)
{
	if (Parent.IsValid())
	{
		Parent->SetHipsDirection(NewHipsDirection);
	}
}

void UAlsLinkedAnimationInstance::ActivatePivot()
{
	if (Parent.IsValid())
	{
		Parent->ActivatePivot();
	}
}

void UAlsLinkedAnimationInstance::ResetJumped()
{
	if (Parent.IsValid())
	{
		Parent->ResetJumped();
	}
}
