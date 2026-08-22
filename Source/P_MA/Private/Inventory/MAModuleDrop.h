#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MAModuleDrop.generated.h"

class AMAPlayerCharacter;
class UMAHighlightComponent;
class UMAInteractableComponent;
class UMASkillModule;
class UMASkillTooltipWidget;
class UStaticMesh;
class UStaticMeshComponent;
class UWidgetComponent;

USTRUCT(BlueprintType)
struct FMAModuleDropData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drop", meta=(ClampMin="1"))
	int32 ModuleId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drop", meta=(ClampMin="1"))
	int32 Count = 1;

	bool IsValid() const { return ModuleId > 0 && Count > 0; }
};

/** Replicated world representation and pickup transaction for a built skill module. */
UCLASS()
class P_MA_API AMAModuleDrop : public AActor
{
	GENERATED_BODY()

public:
	AMAModuleDrop();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool InitializeDrop(int32 ModuleId, int32 Count = 1);

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UStaticMeshComponent> DropMeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAHighlightComponent> HighlightComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UStaticMeshComponent> RarityVisualComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UWidgetComponent> TooltipWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TObjectPtr<UStaticMesh> ModuleMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TObjectPtr<UStaticMesh> SubModuleMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TObjectPtr<UStaticMesh> DefaultItemMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing=OnRep_DropData, Category="Drop")
	FMAModuleDropData DropData;

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void HandleFocus(AMAPlayerCharacter* Interactor, bool bFocused);
	void RefreshPresentation();
	void RefreshTooltip();
	UStaticMesh* ResolveDropMesh() const;

	UFUNCTION()
	void OnRep_DropData();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModule> CachedModule;

	bool bPickupInProgress = false;
};
