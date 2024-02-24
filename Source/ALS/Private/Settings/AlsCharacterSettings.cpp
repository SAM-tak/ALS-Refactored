#include "Settings/AlsCharacterSettings.h"

#include "Utility/AlsGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterSettings)

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
