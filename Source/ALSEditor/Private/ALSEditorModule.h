#pragma once
#include "CoreMinimal.h"

class FALSEditorModule : public FDefaultGameModuleImpl
{
	typedef FALSEditorModule ThisClass;

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};
