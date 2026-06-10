#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "MAAttributeFeedbackComponent.generated.h"

class UMAAbilitySystemComponent;

UCLASS(ClassGroup=(Custom))
class P_MA_API UMAAttributeFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

private:
	void HandleShieldChanged(const FOnAttributeChangeData& Data) const;
	void HandleCoinChanged(const FOnAttributeChangeData& Data) const;
	void ShowPositiveAttributeDelta(float Delta, const FText& Suffix, float ZOffset, const FLinearColor& Color, const FLinearColor& OutlineColor) const;

	TWeakObjectPtr<UMAAbilitySystemComponent> AbilitySystemComponent;
};
