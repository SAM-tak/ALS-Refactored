#include "Settings/AlsCharacterSettings.h"

#include "Utility/AlsGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterSettings)

UAlsCharacterSettings::UAlsCharacterSettings()
{
	Ragdolling.GroundTraceResponseChannels =
	{
		ECC_WorldStatic,
		ECC_WorldDynamic,
		ECC_Destructible
	};

	Ragdolling.GroundTraceResponses.WorldStatic = ECR_Block;
	Ragdolling.GroundTraceResponses.WorldDynamic = ECR_Block;
	Ragdolling.GroundTraceResponses.Destructible = ECR_Block;

	Mantling.MantlingTraceResponseChannels =
	{
		ECC_WorldStatic,
		ECC_WorldDynamic,
		ECC_Destructible
	};

	Mantling.MantlingTraceResponses.WorldStatic = ECR_Block;
	Mantling.MantlingTraceResponses.WorldDynamic = ECR_Block;
	Mantling.MantlingTraceResponses.Destructible = ECR_Block;
}

const FGameplayTag& UAlsCharacterSettings::DesiredToActual(const FGameplayTag& SourceTag) const
{
	if (SourceTag.IsValid())
	{
		auto* Value{GameplayTagCaterogy.DesiredToActualMap.Find(SourceTag)};
		if (Value)
		{
			return *Value;
		}
	}
	return SourceTag;
}

#if WITH_EDITOR
void UAlsCharacterSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, Ragdolling))
	{
		Ragdolling.PostEditChangeProperty(PropertyChangedEvent);
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, Mantling))
	{
		Mantling.PostEditChangeProperty(PropertyChangedEvent);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
