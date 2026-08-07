#include "GAS/Skill/Action/MASkillAction_PatchDamagePayload.h"

#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_PatchDamagePayload::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Scopes);
	if (!DamagePayloadTag.IsValid()) return;

	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;

	TArray<TPair<FGameplayTag, FMASkillDamageConfig>> DamagePayloads;
	Payloads.Reader.FindStructsByTag(DamagePayloadTag, bExactPayloadTagMatch, DamagePayloads);
	if (DamagePayloads.IsEmpty()) return;
	if (bOverrideDamageType && !DamageTypeTag.IsValid()) return;

	for (TPair<FGameplayTag, FMASkillDamageConfig>& DamagePayload : DamagePayloads)
	{
		FMASkillDamageConfig& DamageConfig = DamagePayload.Value;
		if (RequiredDamageTypeTag.IsValid()
			&& DamageConfig.DamageTypeTag != RequiredDamageTypeTag)
		{
			continue;
		}

		DamageConfig.Scale(DamageMultiplier);

		if (bOverrideDamageType)
		{
			DamageConfig.DamageTypeTag = DamageTypeTag;
		}
		if (bOverrideApplicationMode)
		{
			DamageConfig.ApplicationMode = ApplicationMode;
			if (ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime)
			{
				DamageConfig.DamageOverTime = DamageOverTime;
			}
		}

		TargetRelationModifier.ApplyTo(DamageConfig.TargetRelationMask);

		switch (GameplayCuePatchOp)
		{
		case EMASkillGameplayCuePatchOp::None:
			break;
		case EMASkillGameplayCuePatchOp::Append:
			DamageConfig.TargetGameplayCueTags.AppendTags(TargetGameplayCueTags);
			break;
		case EMASkillGameplayCuePatchOp::Replace:
			DamageConfig.TargetGameplayCueTags = TargetGameplayCueTags;
			break;
		default:
			break;
		}

		switch (StatusEffectPatchOp)
		{
		case EMASkillStatusEffectPatchOp::None:
			break;
		case EMASkillStatusEffectPatchOp::Append:
			DamageConfig.StatusEffects.Append(StatusEffects);
			break;
		case EMASkillStatusEffectPatchOp::Replace:
			DamageConfig.StatusEffects = StatusEffects;
			break;
		default:
			break;
		}

		Payloads.Writer.SetStruct(EMASkillPayloadScope::Skill, DamagePayload.Key, DamageConfig);
	}
}
