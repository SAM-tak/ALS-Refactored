#include "Components/AlsOverlayModeComponent.h"

#include "AlsCharacter.h"
#include "LinkedAnimLayers/AlsOverlayAnimInstance.h"
#include "Utility/AlsMath.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsOverlayModeComponent)

void UAlsOverlayModeComponent::OnRefresh_Implementation(float DeltaTime)
{
	auto OverlayMode{Character->GetOverlayMode()};
	if (OverlayMode != CurrentOverlayMode)
	{
		OnChangeOverlayMode(OverlayMode);
		CurrentOverlayMode = OverlayMode;
	}
}

void UAlsOverlayModeComponent::OnChangeOverlayMode_Implementation(const FGameplayTag& NewOverlayMode) {}

void UAlsOverlayModeComponent::LinkAnimLayer(const FGameplayTag& Tag)
{
	if (Tag.IsValid() && OverlayAnimLayerMap.Contains(Tag))
	{
		Character->GetMesh()->LinkAnimClassLayers(OverlayAnimLayerMap[Tag]);
	}
	else if(CurrentOverlayMode.IsValid() && OverlayAnimLayerMap.Contains(CurrentOverlayMode))
	{
		Character->GetMesh()->UnlinkAnimClassLayers(OverlayAnimLayerMap[CurrentOverlayMode]);
	}
}
