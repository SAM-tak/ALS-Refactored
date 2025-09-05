#include "AlsCharacterComponent.h"

#include "Misc/UObjectToken.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AlsCharacter.h"
#include "Utility/AlsLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterComponent)

UAlsCharacterComponent::UAlsCharacterComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bTickInEditor = false;
}

void UAlsCharacterComponent::OnRegister()
{
	Super::OnRegister();

	Character = GetPawn<AAlsCharacter>();

	if (!Character.IsValid())
	{
		UE_LOG(LogAls, Error, TEXT("[UAlsCharacterComponent::OnRegister] This component has been added to a blueprint whose base class is not a child of AlsCharacter. To use this component, it MUST be placed on a child of AlsCharacter Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("AlsCharacterComponent", "NotOnAlsCharacterError", "has been added to a blueprint whose base class is not a child of AlsCharacter. To use this component, it MUST be placed on a child of AlsCharacter Blueprint. This will cause a crash if you PIE!");
			static const FName MessageLogName = TEXT("AlsCharacterComponent");
			
			FMessageLog(MessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
			
			FMessageLog(MessageLogName).Open();
		}
#endif
	}
	else
	{
		Character->OnPossessed_Client.AddUObject(this, &ThisClass::OnPossessed);
		Character->OnUnPossessed_Client.AddUObject(this, &ThisClass::OnUnPossessed);
		Character->OnRefresh.AddUObject(this, &ThisClass::OnRefresh);
	}
}

void UAlsCharacterComponent::OnPossessed_Implementation(AController* NewController) {}

void UAlsCharacterComponent::OnUnPossessed_Implementation(AController* PreviousController) {}

void UAlsCharacterComponent::OnRefresh_Implementation(float DeltaTime) {}
