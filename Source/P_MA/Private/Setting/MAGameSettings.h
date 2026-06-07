#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MAGameSettings.generated.h"

class UUserWidget;
class UMASkillGenericDataAsset;
class UMaterialInterface;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="MA Game Settings"))
class P_MA_API UMAGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UMAGameSettings* Get() { return GetDefault<UMAGameSettings>(); }
	const UMASkillGenericDataAsset* GetDefaultSkillGenericDataAsset() const;
	UMaterialInterface* GetOverlayMaterial() const;

	UPROPERTY(Config, EditAnywhere, Category="Interact")
	TSoftClassPtr<UUserWidget> DefaultInteractKeyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Skill")
	TSoftObjectPtr<UMASkillGenericDataAsset> DefaultSkillGenericDataAsset;

	UPROPERTY(Config, EditAnywhere, Category="Visual|Overlay")
	TSoftObjectPtr<UMaterialInterface> OverlayMaterial;
};
