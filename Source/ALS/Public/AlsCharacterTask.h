#pragma once

#include "CoreMinimal.h"
#include "AlsCharacterTask.generated.h"

class UWorld;
class AActor;
class UAlsCharacterComponent;

UCLASS(Abstract, Blueprintable, BlueprintType, AutoExpandCategories = ("Settings"))
class ALS_API UAlsCharacterTask : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	uint8 bEnableInputBinding : 1{true};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TWeakObjectPtr<AAlsCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bActive : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInputBinded : 1{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bEpilogRunningCurrently : 1{false};

public:
	virtual bool IsActive() const { return bActive; }

	virtual void OnRegister();

	virtual void Begin();

	virtual void Refresh(float DeltaTime);

	virtual void OnControllerChanged(AController* PreviousController, AController* NewController);

	void End();

	void Cancel();

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "ALS|CharacterTask")
	bool IsEpilogRunning() const;

	bool HasFinished() const
	{
		return !bActive && !bEpilogRunningCurrently;
	}

	virtual UWorld* GetWorld() const override final;

protected:
	virtual void OnEnd(bool bWasCancelled);

	virtual void OnFinished();

	void BindInput(UInputComponent* InputComponent);

	void UnbindInput(UInputComponent* InputComponent);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|CharacterTask", DisplayName = "OnBegin", Meta = (ScriptName = "OnBegin"))
	void K2_OnBegin();

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|CharacterTask", DisplayName = "OnRefresh", Meta = (ScriptName = "OnRefresh"))
	void K2_OnRefresh(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|CharacterTask", DisplayName = "OnEnd", Meta = (ScriptName = "OnEnd"))
	void K2_OnEnd(bool bWasCancelled);

	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|CharacterTask", DisplayName = "OnFinished", Meta = (ScriptName = "OnFinished"))
	void K2_OnFinished();
};
