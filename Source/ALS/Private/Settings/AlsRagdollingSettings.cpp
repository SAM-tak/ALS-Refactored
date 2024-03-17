#include "Settings/AlsRagdollingSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsRagdollingSettings)

UAlsRagdollingSettings::UAlsRagdollingSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GroundTraceResponses.WorldStatic = ECR_Block;
	GroundTraceResponses.WorldDynamic = ECR_Block;
	GroundTraceResponses.Destructible = ECR_Block;
}

#if WITH_EDITOR
void UAlsRagdollingSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(ThisClass, GroundTraceResponseChannels))
	{
		return;
	}

	GroundTraceResponses.SetAllChannels(ECR_Ignore);

	for (const auto& CollisionChannel : GroundTraceResponseChannels)
	{
		GroundTraceResponses.SetResponse(CollisionChannel, ECR_Block);
	}
}
#endif
