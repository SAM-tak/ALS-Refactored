#include "ALSEditorModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Editor/UnrealEdEngine.h"
#include "ClassIconFinder.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "ALSEditor"

namespace {
	TSharedPtr<FSlateStyleSet> StyleSetInstance = nullptr;
}

#define SLATE_IMAGE_SVG(RelativePath, Size) \
    new FSlateVectorImageBrush(StyleSetInstance->RootToContentDir(RelativePath, TEXT(".svg")), FVector2D(Size, Size))

void FALSEditorModule::StartupModule()
{
	StyleSetInstance = MakeShareable(new FSlateStyleSet("ALSEditorStyle"));

	StyleSetInstance->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("ALS"))->GetContentDir());

	StyleSetInstance->Set("ClassIcon.AlsCameraMovementComponent", SLATE_IMAGE_SVG("Icons/AlsCamera_16", 16.0f));
	StyleSetInstance->Set("ClassThumbnail.AlsCameraMovementComponent", SLATE_IMAGE_SVG("Icons/AlsCamera_64", 64.0f));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);
}

void FALSEditorModule::ShutdownModule()
{
	// Unregister the style
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance);
	StyleSetInstance.Reset();
}

IMPLEMENT_MODULE(FALSEditorModule, ALSEditor);

#undef LOCTEXT_NAMESPACE
