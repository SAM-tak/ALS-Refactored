#pragma once

#include "AlsCharacterComponent.h"
#include "GameplayTags.h"
#include "AlsLocalMontageComponent.generated.h"

class UAlsLocalMontageTask;

UCLASS(Abstract, AutoExpandCategories = ("AlsLocalMontageModeComponent|Settings"))
class ALS_API UAlsLocalMontageComponent : public UAlsCharacterComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AlsLocalMontageModeComponent|Settings", Meta = (DisplayThumbnail = false))
	TMap<FGameplayTag, TSubclassOf<UAlsLocalMontageTask>> LocalMontageTaskClassMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsLocalMontageModeComponent|State", Transient)
	FGameplayTag CurrentLocalMontageTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsLocalMontageModeComponent|State", Transient)
	TWeakObjectPtr<UAlsLocalMontageTask> CurrentLocalMontageTask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsLocalMontageModeComponent|State", Transient)
	FGameplayTagContainer LocalMontageTagsMask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AlsLocalMontageModeComponent|State", Transient)
	TMap<FGameplayTag, TObjectPtr<UAlsLocalMontageTask>> InstancedLocalMontageTasks;

public:
	UAlsLocalMontageTask* Play(const FGameplayTag& LocalMontageTag);

	void OnEndTask(class UAlsLocalMontageTask *Task);

protected:
	virtual void BeginPlay() override;

	virtual void OnRefresh_Implementation(float DeltaTime) override;

	virtual void OnControllerChanged_Implementation(AController* PreviousController, AController* NewController) override;

	UFUNCTION(BlueprintCallable, Category = "AlsLocalMontageModeComponent")
	void AddOrUpdateReplicatedWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector TargetLocation, FRotator TargetRotation);

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlay(const FGameplayTag& LocalMontageTag);

	UAlsLocalMontageTask* PlayImplementation(const FGameplayTag& LocalMontageTag);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAddOrUpdateWarpTargetFromLocationAndRotation(FName WarpTargetName, FVector_NetQuantize TargetLocation, FRotator TargetRotation);
};
