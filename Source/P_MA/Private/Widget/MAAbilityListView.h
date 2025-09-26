// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "Gas/MAGameplayAbilityTypes.h"
#include "MAAbilityListView.generated.h"

/**
 * 
 */
UCLASS()
class UMAAbilityListView : public UListView
{
	GENERATED_BODY()

public:
	void ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);

private:
	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* AbilityDataTable;

	void AbilityGaugeGenerated(UUserWidget& Widget);

	const struct FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

};