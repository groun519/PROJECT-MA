#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAGameSettings.generated.h"

class UUserWidget;
class UDataTable;
class AMAModuleDrop;
class UPA_AbilitySystemGenerics;
class UMAModuleQualityData;
class UMAElementalConfigData;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FMADamageTextStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Damage Text")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category="Damage Text")
	FLinearColor OutlineColor = FLinearColor::Transparent;

	UPROPERTY(EditAnywhere, Category="Damage Text")
	float ZOffset = 100.f;
};

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="MA Game Settings"))
class P_MA_API UMAGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMAGameSettings();
	static const UMAGameSettings* Get() { return GetDefault<UMAGameSettings>(); }
	const UPA_AbilitySystemGenerics* GetAbilitySystemGenerics() const;
	const UDataTable* GetPlayerBaseStatDataTable() const;
	const UDataTable* GetMonsterBaseStatDataTable() const;
	const UDataTable* GetElementalDataTable() const;
	const UDataTable* GetAreaDecalDataTable() const;
	const UDataTable* GetWarningTextDataTable() const;
	const UMAModuleQualityData* GetModuleQualityData() const;
	const UMAElementalConfigData* GetElementalConfigData() const;
	UMaterialInterface* GetOverlayMaterial() const;
	UMaterialInterface* GetSpaceTransitionMaterial() const;
	FMADamageTextStyle GetDamageTextStyle(const FGameplayTag& DamageTypeTag, EMADamageCriticalResult CriticalResult, bool bIsPlayerHit) const;

	UPROPERTY(Config, EditAnywhere, Category="Interact")
	TSoftClassPtr<UUserWidget> DefaultInteractKeyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Inventory|Drop")
	TSoftClassPtr<AMAModuleDrop> ModuleDropActorClass;

	UPROPERTY(Config, EditAnywhere, Category="Gameplay Ability")
	TSoftObjectPtr<UPA_AbilitySystemGenerics> AbilitySystemGenerics;

	UPROPERTY(Config, EditAnywhere, Category="Base Stats")
	TSoftObjectPtr<UDataTable> PlayerBaseStatDataTable;

	UPROPERTY(Config, EditAnywhere, Category="Base Stats")
	TSoftObjectPtr<UDataTable> MonsterBaseStatDataTable;

	UPROPERTY(Config, EditAnywhere, Category="Skill")
	TSoftObjectPtr<UMAModuleQualityData> ModuleQualityData;

	UPROPERTY(Config, EditAnywhere, Category="Skill|Module", meta=(ClampMin="0"))
	int32 MaxEnchantmentsPerModule = 3;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Config, EditAnywhere, Category="Skill|Module")
	FDirectoryPath SkillModuleJsonDirectory;
#endif

	UPROPERTY(Config, EditAnywhere, Category="Elemental")
	TSoftObjectPtr<UMAElementalConfigData> ElementalConfigData;

	UPROPERTY(Config, EditAnywhere, Category="Elemental", meta=(RowType="/Script/P_MA.MAElementDataRow"))
	TSoftObjectPtr<UDataTable> ElementalDataTable;

	UPROPERTY(Config, EditAnywhere, Category="Skill|Area", meta=(RowType="/Script/P_MA.MAAreaDecalDataRow"))
	TSoftObjectPtr<UDataTable> AreaDecalDataTable;

	UPROPERTY(Config, EditAnywhere, Category="Skill|Warning", meta=(RowType="/Script/P_MA.MASkillWarningTextDataRow"))
	TSoftObjectPtr<UDataTable> WarningTextDataTable;

	UPROPERTY(Config, EditAnywhere, Category="Skill|Cooldown")
	FLinearColor PositiveCooldownColor = FLinearColor::White;

	UPROPERTY(Config, EditAnywhere, Category="Skill|Cooldown")
	FLinearColor NegativeCooldownColor = FLinearColor(0.25f, 0.75f, 1.f, 1.f);

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle DefaultDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle PlayerHitDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle CriticalDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle ReverseCriticalDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle HealDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle FireDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FMADamageTextStyle IceDamageTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Damage Text")
	FLinearColor FixedDamageOutlineColor = FLinearColor(0.82f, 0.82f, 0.78f, 1.f);

	UPROPERTY(Config, EditAnywhere, Category="Attribute Feedback")
	FMADamageTextStyle ShieldAttributeFeedbackTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Attribute Feedback")
	FMADamageTextStyle CoinAttributeFeedbackTextStyle;

	UPROPERTY(Config, EditAnywhere, Category="Visual|Overlay")
	TSoftObjectPtr<UMaterialInterface> OverlayMaterial;

	UPROPERTY(Config, EditAnywhere, Category="Visual|Transition")
	TSoftObjectPtr<UMaterialInterface> SpaceTransitionMaterial;
};
