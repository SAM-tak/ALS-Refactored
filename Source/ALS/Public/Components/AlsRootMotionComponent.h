#pragma once

#include "AlsCharacterComponent.h"
#include "AlsRootMotionComponent.generated.h"

struct FAlsRootMotionSource_Mantling;

UENUM(BlueprintType)
enum class EAlsMantlingType : uint8
{
	High,
	Medium,
	Low,
	InAir
};

USTRUCT(BlueprintType)
struct ALS_API FAlsMantlingParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FVector_NetQuantize100 TargetRelativeLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FRotator TargetRelativeRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "cm"))
	float MantlingHeight{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	EAlsMantlingType MantlingType{EAlsMantlingType::High};
};

UCLASS(Blueprintable, BlueprintType)
class ALS_API UAlsMantlingSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAnimMontage> Montage;

	// Optional mantling time to horizontal correction amount curve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UCurveFloat> HorizontalCorrectionCurve;

	// Optional mantling time to vertical correction amount curve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UCurveFloat> VerticalCorrectionCurve;
};

UCLASS(Abstract, AutoExpandCategories = ("AlsRootMotionComponent|Settings"))
class ALS_API UAlsRootMotionComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRefresh_Implementation(float DeltaTime) override;

	// Mantling

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Root Motion Component|Mantling")
	void StartMantling(const FAlsMantlingParameters& Parameters);

	FAlsRootMotionSource_Mantling* GetCurrentMantlingRootMotionSource() const;

	UAnimMontage* GetMantlingMontage() const;

	bool IsMantlingActive() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "ALS|Root Motion Component|Mantling")
	UAlsMantlingSettings* SelectMantlingSettings(const FAlsMantlingParameters& Parameters) const;

	void OnTick_Mantling();

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartMantling(const FAlsMantlingParameters& Parameters);

	void StartMantlingImplementation(const FAlsMantlingParameters& Parameters);

	float CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, const float MantlingHeight) const;

	UPROPERTY(VisibleAnywhere, Category = "Mantling|State", Transient)
	TWeakObjectPtr<UAnimMontage> MantlingMontage;

	UPROPERTY(VisibleAnywhere, Category = "Mantling|State", Transient)
	int32 MantlingRootMotionSourceId{0};
};

inline UAnimMontage* UAlsRootMotionComponent::GetMantlingMontage() const
{
	return MantlingMontage.Get();
}

inline bool UAlsRootMotionComponent::IsMantlingActive() const
{
	return MantlingMontage.IsValid() && MantlingRootMotionSourceId > 0;
}
