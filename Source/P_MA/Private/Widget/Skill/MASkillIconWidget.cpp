#include "Widget/Skill/MASkillIconWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "MAMaterialParams.h"
#include "GameplayEffect.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

void UMASkillIconWidget::SetHotkeyText(const FText& InText)
{
	if (!HotkeyText) return;

	HotkeyText->SetText(InText);
	HotkeyText->SetVisibility(InText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

void UMASkillIconWidget::SetSkillModule(const UMASkillModule* SkillModule)
{
	if (!SkillIconImage) return;

	const FMAIconData IconData = SkillModule
		? SkillModule->ResolveDisplayData(UMAGameSettings::Get()->GetModuleQualityData()).IconData
		: FMAIconData();

	if (UMaterialInstanceDynamic* IconMaterial = SkillIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, IconData.SubIcon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, IconData.Icon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, IconData.SubIcon ? 1.f : 0.f);
	}
	else if (IconData.Icon)
	{
		SkillIconImage->SetBrushFromTexture(IconData.Icon);
	}
	else
	{
		SkillIconImage->SetBrush(FSlateBrush());
	}

	SkillIconImage->SetVisibility(ESlateVisibility::Visible);
	RefreshTooltip(SkillModule);
}

void UMASkillIconWidget::SetCooldownTag(FGameplayTag InCooldownTag)
{
	CooldownTag = InCooldownTag;
	RefreshCooldown();
}

void UMASkillIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshCooldown();
}

void UMASkillIconWidget::RefreshTooltip(const UMASkillModule* SkillModule)
{
	if (!SkillModule || !TooltipWidgetClass)
	{
		SetToolTip(nullptr);
		return;
	}

	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget)
	{
		SetToolTip(nullptr);
		return;
	}

	TooltipWidget->SetSkillTooltip(SkillModule);
	SetToolTip(TooltipWidget);
}

void UMASkillIconWidget::RefreshCooldown()
{
	if (!CooldownTag.IsValid())
	{
		SetCooldownDisplay(0.f, 0.f);
		return;
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	const UAbilitySystemComponent* AbilitySystemComponent = OwningPawn
		? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningPawn)
		: nullptr;
	if (!AbilitySystemComponent)
	{
		SetCooldownDisplay(0.f, 0.f);
		return;
	}

	const FGameplayEffectQuery CooldownQuery(FActiveGameplayEffectQueryCustomMatch::CreateLambda(
		[CooldownTag = CooldownTag](const FActiveGameplayEffect& ActiveEffect)
		{
			return ActiveEffect.Spec.DynamicGrantedTags.HasTagExact(CooldownTag);
		}));
	const TArray<TPair<float, float>> CooldownTimes = AbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	float RemainingSeconds = 0.f;
	float DurationSeconds = 0.f;
	for (const TPair<float, float>& CooldownTime : CooldownTimes)
	{
		if (CooldownTime.Key <= RemainingSeconds) continue;

		RemainingSeconds = CooldownTime.Key;
		DurationSeconds = CooldownTime.Value;
	}

	SetCooldownDisplay(RemainingSeconds, DurationSeconds);
}

void UMASkillIconWidget::SetCooldownDisplay(float RemainingSeconds, float DurationSeconds)
{
	const float CooldownAlpha = DurationSeconds > 0.f
		? FMath::Clamp(1.f - RemainingSeconds / DurationSeconds, 0.f, 1.f)
		: 1.f;

	if (CooldownOverlayImage)
	{
		CooldownOverlayImage->SetVisibility(RemainingSeconds > 0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (UMaterialInstanceDynamic* CooldownMaterial = CooldownOverlayImage->GetDynamicMaterial())
		{
			CooldownMaterial->SetScalarParameterValue(PARAM_ModuleIcon_CooldownAlpha, CooldownAlpha);
		}
	}

	if (!CooldownText) return;

	CooldownText->SetVisibility(RemainingSeconds > 0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (RemainingSeconds > 0.f)
	{
		CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(RemainingSeconds)));
	}
}
