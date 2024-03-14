#include "LinkedAnimLayers/AlsLayeringAnimInstance.h"
#include "AlsAnimationInstance.h"
#include "AlsAnimationInstanceProxy.h"
#include "AlsCharacter.h"
#include "Utility/AlsConstants.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsLayeringAnimInstance)

void UAlsLayeringAnimInstance::Refresh()
{
	const auto& Curves{GetAnimationCurvesFromProxy(EAnimCurveType::AttributeCurve)};

	static const auto GetCurveValue{[](const TMap<FName, float>& Curves, const FName& CurveName) -> float
		{
			const auto* Value{Curves.Find(CurveName)};

			return Value != nullptr ? *Value : 0.0f;
		}};

	HeadBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerHeadCurveName());
	HeadAdditiveBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerHeadAdditiveCurveName());
	HeadSlotBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerHeadSlotCurveName());

	// The mesh space blend will always be 1 unless the local space blend is 1.

	ArmLeftBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmLeftCurveName());
	ArmLeftAdditiveBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmLeftAdditiveCurveName());
	ArmLeftSlotBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmLeftSlotCurveName());
	ArmLeftLocalSpaceBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmLeftLocalSpaceCurveName());
	ArmLeftMeshSpaceBlendAmount = !FAnimWeight::IsFullWeight(ArmLeftLocalSpaceBlendAmount);

	// The mesh space blend will always be 1 unless the local space blend is 1.

	ArmRightBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmRightCurveName());
	ArmRightAdditiveBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmRightAdditiveCurveName());
	ArmRightSlotBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmRightSlotCurveName());
	ArmRightLocalSpaceBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerArmRightLocalSpaceCurveName());
	ArmRightMeshSpaceBlendAmount = !FAnimWeight::IsFullWeight(ArmRightLocalSpaceBlendAmount);

	HandLeftBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerHandLeftCurveName());
	HandRightBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerHandRightCurveName());

	SpineBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerSpineCurveName());
	SpineAdditiveBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerSpineAdditiveCurveName());
	SpineSlotBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerSpineSlotCurveName());

	PelvisBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerPelvisCurveName());
	PelvisSlotBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerPelvisSlotCurveName());

	LegsBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerLegsCurveName());
	LegsSlotBlendAmount = GetCurveValue(Curves, UAlsConstants::LayerLegsSlotCurveName());
}
