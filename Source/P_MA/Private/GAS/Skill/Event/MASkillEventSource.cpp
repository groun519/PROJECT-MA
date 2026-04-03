#include "GAS/Skill/Event/MASkillEventSource.h"

#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillEventSource::EmitEvent() const
{
	if (RuntimeContext)
	{
		RuntimeContext->HandleTagEvent(EmittedTag);
	}
}
