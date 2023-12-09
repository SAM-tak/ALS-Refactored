#include "ALSEditorModule.h"
#include "ClassIconFinder.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"
#include "Editor/UnrealEdEngine.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

#define SLATE_IMAGE_BRUSH(ImagePath, ImageSize) new FSlateImageBrush(StyleSetInstance->RootToContentDir(TEXT(ImagePath), TEXT(".png")), FVector2D(ImageSize, ImageSize))
#define SLATE_IMAGE_BRUSH_SVG(ImagePath, ImageSize) new FSlateVectorImageBrush(StyleSetInstance->RootToContentDir(TEXT(ImagePath), TEXT(".svg")), FVector2D(ImageSize, ImageSize))

/**
 * ALS Editor Module
 */
class FALSEditorModule : public FDefaultModuleImpl
{
	typedef FALSEditorModule ThisClass;

	static inline TSharedPtr<FSlateStyleSet> StyleSetInstance = nullptr;

	virtual void StartupModule() override
	{
		// Create the new style set
		StyleSetInstance = MakeShareable(new FSlateStyleSet("ALSEditorModuleStyle"));
		// Assign the content root of this style set
		StyleSetInstance->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
		StyleSetInstance->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
		// Modify the AlsCameraComponent class icon to camera component's icon
		StyleSetInstance->Set("ClassIcon.AlsCameraComponent", SLATE_IMAGE_BRUSH("Icons/AssetIcons/CameraActor_16x", 16.0f));
		StyleSetInstance->Set("ClassIcon.AlsCameraComponent", SLATE_IMAGE_BRUSH_SVG("Starship/AssetIcons/CameraActor_16", 16.0f));
		// Finally register the style set so it is actually used
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);
	}

	virtual void ShutdownModule() override
	{
		FModuleManager::Get().OnModulesChanged().RemoveAll(this);

		// Unregister the style set and reset the pointer
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance.Get());
		StyleSetInstance.Reset();
	}
};

IMPLEMENT_MODULE(FALSEditorModule, ALSEditor);
