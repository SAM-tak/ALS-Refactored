#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AlsCameraConstants.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class ALSCAMERA_API UAlsCameraConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Debug

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CameraCurvesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category = "ALS|Camera Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CameraTracesDebugDisplayName();
};

inline const FName& UAlsCameraConstants::CameraCurvesDebugDisplayName()
{
	static const FName Name{TEXTVIEW("ALS.CameraCurves")};
	return Name;
}

inline const FName& UAlsCameraConstants::CameraTracesDebugDisplayName()
{
	static const FName Name{TEXTVIEW("ALS.CameraTraces")};
	return Name;
}
