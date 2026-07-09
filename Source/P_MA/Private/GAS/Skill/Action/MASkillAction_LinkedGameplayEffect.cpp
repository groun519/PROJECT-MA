#include "GAS/Skill/Action/MASkillAction_LinkedGameplayEffect.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

static FGameplayTag ResolveHandlePayloadTag(FGameplayTag HandlePayloadTag)
{
	return HandlePayloadTag.IsValid()
		? HandlePayloadTag
		: UMAAbilitySystemStatics::GetModuleLinkedGameplayEffectHandleTag();
}

static bool TryGetLinkedHandle(
	const FMASkillPayloadAccessor& Payloads,
	FGameplayTag HandlePayloadTag,
	FActiveGameplayEffectHandle& OutHandle)
{
	OutHandle = FActiveGameplayEffectHandle();
	return Payloads.TryGetStruct(ResolveHandlePayloadTag(HandlePayloadTag), OutHandle)
		&& OutHandle.IsValid();
}

static void SetLinkedHandle(
	FMASkillPayloadAccessor& Payloads,
	FGameplayTag HandlePayloadTag,
	FActiveGameplayEffectHandle Handle)
{
	Payloads.SetStruct(
		EMASkillPayloadWriteScope::Module,
		ResolveHandlePayloadTag(HandlePayloadTag),
		Handle);
}

static bool IsLinkedEffectClass(
	const UAbilitySystemComponent& AbilitySystemComponent,
	FActiveGameplayEffectHandle EffectHandle,
	const TSubclassOf<UGameplayEffect>& GameplayEffectClass)
{
	const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent.GetActiveGameplayEffect(EffectHandle);
	return ActiveEffect
		&& ActiveEffect->Spec.Def
		&& ActiveEffect->Spec.Def->GetClass() == GameplayEffectClass;
}

static void ResolveSetByCallers(
	const TArray<FMASkillLinkedGameplayEffectSetByCaller>& SetByCallers,
	const FMASkillPayloadAccessor& Payloads,
	TMap<FGameplayTag, float>& OutValues)
{
	OutValues.Reset();

	for (const FMASkillLinkedGameplayEffectSetByCaller& SetByCaller : SetByCallers)
	{
		float Value = 0.f;
		if (SetByCaller.TryResolve(Payloads, Value))
		{
			OutValues.FindOrAdd(SetByCaller.SetByCallerTag) = Value;
		}
	}
}

bool FMASkillLinkedGameplayEffectSetByCaller::TryResolve(
	const FMASkillPayloadAccessor& Payloads,
	float& OutValue) const
{
	OutValue = BaseValue;
	if (!SetByCallerTag.IsValid()) return false;
	if (!PayloadTag.IsValid()) return true;

	float PayloadValue = 0.f;
	if (!Payloads.TryGetScalar(PayloadTag, PayloadValue)) return false;

	OutValue += PayloadValue * PayloadMultiplier;
	return true;
}

void UMASkillAction_LinkModuleGameplayEffect::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority() || !GameplayEffectClass || !Scopes.Module) return;

	UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	UMASkillRuntimeRegistry* RuntimeRegistry = Scopes.Skill ? Scopes.Skill->GetRuntimeRegistry() : nullptr;
	if (!RuntimeRegistry) return;

	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	FActiveGameplayEffectHandle EffectHandle;
	if (TryGetLinkedHandle(Payloads, HandlePayloadTag, EffectHandle))
	{
		if (IsLinkedEffectClass(*AbilitySystemComponent, EffectHandle, GameplayEffectClass))
		{
			TMap<FGameplayTag, float> SetByCallerValues;
			ResolveSetByCallers(SetByCallers, Payloads, SetByCallerValues);
			if (!SetByCallerValues.IsEmpty())
			{
				AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitudes(
					EffectHandle,
					SetByCallerValues);
			}
			return;
		}

		AbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle);
		SetLinkedHandle(Payloads, HandlePayloadTag, FActiveGameplayEffectHandle());
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(&OwnerAbility);

	FGameplayEffectSpecHandle SpecHandle =
		AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, EffectLevel, EffectContext);
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid()) return;

	TMap<FGameplayTag, float> SetByCallerValues;
	ResolveSetByCallers(SetByCallers, Payloads, SetByCallerValues);
	for (const TPair<FGameplayTag, float>& Pair : SetByCallerValues)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
	}

	EffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (!EffectHandle.IsValid()) return;

	SetLinkedHandle(Payloads, HandlePayloadTag, EffectHandle);
	RuntimeRegistry->Register(AbilitySystemComponent, EffectHandle);
}

void UMASkillAction_ModifyLinkedGameplayEffect::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority() || SetByCallers.IsEmpty()) return;

	UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	FActiveGameplayEffectHandle EffectHandle;
	if (!TryGetLinkedHandle(Payloads, HandlePayloadTag, EffectHandle)) return;
	if (!AbilitySystemComponent->GetActiveGameplayEffect(EffectHandle)) return;

	TMap<FGameplayTag, float> SetByCallerValues;
	ResolveSetByCallers(SetByCallers, Payloads, SetByCallerValues);
	if (SetByCallerValues.IsEmpty()) return;

	AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitudes(
		EffectHandle,
		SetByCallerValues);
}
