#include "GAS/Skill/Payload/MASkillPayloadEntry.h"

#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void FMASkillPayloadEntry::ApplyTo(FMASkillPayloadStore& PayloadStore) const
{
	if (!PayloadTag.IsValid()) return;

	switch (ValueType)
	{
	case EMASkillPayloadValueType::Scalar:
		PayloadStore.SetScalar(PayloadTag, ScalarValue);
		break;
	case EMASkillPayloadValueType::Vector:
		PayloadStore.SetVector(PayloadTag, VectorValue);
		break;
	case EMASkillPayloadValueType::Object:
		PayloadStore.SetObject(PayloadTag, ObjectValue);
		break;
	case EMASkillPayloadValueType::Struct:
		PayloadStore.SetStructValue(PayloadTag, StructValue);
		break;
	default:
		break;
	}
}

