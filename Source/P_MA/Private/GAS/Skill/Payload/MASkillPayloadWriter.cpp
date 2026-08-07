#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

FMASkillPayloadWriter::FMASkillPayloadWriter(
	FMASkillPayloadStore* InSkillPayloads,
	FMASkillPayloadStore* InModulePayloads)
	: SkillPayloads(InSkillPayloads)
	, ModulePayloads(InModulePayloads)
{
}

void FMASkillPayloadWriter::SetScalar(
	EMASkillPayloadScope Scope,
	const FGameplayTag& Key,
	float Value)
{
	if (FMASkillPayloadStore* Store = GetStore(Scope)) Store->SetScalar(Key, Value);
}

bool FMASkillPayloadWriter::AddScalar(
	EMASkillPayloadScope Scope,
	const FGameplayTag& Key,
	float Value,
	TOptional<float> MinimumValue)
{
	FMASkillPayloadStore* Store = GetStore(Scope);
	return Store && Store->AddScalar(Key, Value, MinimumValue);
}

bool FMASkillPayloadWriter::MultiplyScalar(
	EMASkillPayloadScope Scope,
	const FGameplayTag& Key,
	float Value)
{
	FMASkillPayloadStore* Store = GetStore(Scope);
	return Store && Store->MultiplyScalar(Key, Value);
}

void FMASkillPayloadWriter::SetVector(
	EMASkillPayloadScope Scope,
	const FGameplayTag& Key,
	const FVector& Value)
{
	if (FMASkillPayloadStore* Store = GetStore(Scope)) Store->SetVector(Key, Value);
}

void FMASkillPayloadWriter::SetObject(
	EMASkillPayloadScope Scope,
	const FGameplayTag& Key,
	UObject* Value)
{
	if (FMASkillPayloadStore* Store = GetStore(Scope)) Store->SetObject(Key, Value);
}

FMASkillPayloadStore* FMASkillPayloadWriter::GetStore(EMASkillPayloadScope Scope) const
{
	return Scope == EMASkillPayloadScope::Skill ? SkillPayloads : ModulePayloads;
}
