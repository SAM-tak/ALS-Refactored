#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AlsAbilitySystemComponent.generated.h"

/**
 * AbilitySystemComponent for ALS Refactored
 */
UCLASS()
class ALS_API UAlsAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_UCLASS_BODY()

public:
	inline void CancelAbilitiesBySingleTag(const FGameplayTag& Tag)
	{
		FGameplayTagContainer TagContainer{Tag};
		CancelAbilities(&TagContainer);
	}

	inline bool TryActivateAbilitiesBySingleTag(const FGameplayTag& Tag, bool bAllowRemoteActivation = true)
	{
		return TryActivateAbilitiesByTag(FGameplayTagContainer{Tag}, bAllowRemoteActivation);
	}
};
