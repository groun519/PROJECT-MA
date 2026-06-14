#include "GAS/Skill/Module/MASkillModuleInstance.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "Net/UnrealNetwork.h"

UMASkillModuleInstance* UMASkillModuleInstance::Create(UObject* Outer, UMASkillDefinition* InDefinition)
{
	if (!Outer || !InDefinition) return nullptr;

	UMASkillModuleInstance* Instance = NewObject<UMASkillModuleInstance>(Outer);
	if (!Instance) return nullptr;

	Instance->SetDefinition(InDefinition);
	return Instance;
}

void UMASkillModuleInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMASkillModuleInstance, Definition);
}

void UMASkillModuleInstance::SetDefinition(UMASkillDefinition* InDefinition)
{
	Definition = InDefinition;
	InitializePayloadStore();
}

void UMASkillModuleInstance::OnRep_Definition()
{
	InitializePayloadStore();
}

void UMASkillModuleInstance::InitializePayloadStore()
{
	PayloadStore.Reset();
	if (Definition) Definition->ApplyPayloadsTo(PayloadStore);
}
