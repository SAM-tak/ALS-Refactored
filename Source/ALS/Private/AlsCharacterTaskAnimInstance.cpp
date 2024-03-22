#include "AlsCharacterTaskAnimInstance.h"
#include "AlsCharacterTask.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterTaskAnimInstance)

void UAlsCharacterTaskAnimInstance::Refresh(const UAlsCharacterTask* CharacterTask)
{
	bCharacterTaskActive = CharacterTask->IsActive();
}

void UAlsCharacterTaskAnimInstance::ObserveBlending(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	ObservingFinalBlendWeight = Context.GetContext()->GetFinalBlendWeight();
}
