#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutWeaponModuleButtonWidget.generated.h"

class UButton;
class UImage;
class UMaterialInstanceDynamic;
class UMASkillModule;
class ULoadoutWeaponModuleButtonWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutWeaponModuleSelected, ULoadoutWeaponModuleButtonWidget*);

UCLASS()
class P_MA_API ULoadoutWeaponModuleButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetModule(UMASkillModule* InModule);
	UMASkillModule* GetModule() const { return Module; }
	void SetSelected(bool bSelected);

	FOnLoadoutWeaponModuleSelected OnModuleSelected;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ModuleButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> EquippedBorder;

private:
	UFUNCTION()
	void HandleModuleButtonClicked();
	void CreateIconMaterial();
	void RefreshIcon();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModule> Module;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> IconMaterial;
};
