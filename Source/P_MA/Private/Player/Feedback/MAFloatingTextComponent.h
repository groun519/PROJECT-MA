#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAFloatingTextComponent.generated.h"

class AMAFloatingTextActor;
class APawn;
class UMAAbilitySystemComponent;

UCLASS(ClassGroup=(Feedback), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAFloatingTextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAFloatingTextComponent();

	void BindToPawn(APawn* Pawn);

	UFUNCTION(Client, Unreliable)
	void ClientShowDamage(
		float Amount,
		AActor* TargetActor,
		EMADamageCriticalResult CriticalResult,
		bool bIsIncoming,
		FGameplayTag DamageTypeTag);

	void ShowLocal(
		const FText& Text,
		const FVector& WorldLocation,
		const FLinearColor& Color,
		const FLinearColor& OutlineColor = FLinearColor::Transparent,
		float Scale = 1.f) const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void UnbindASC();
	void HandleShieldChanged(const FOnAttributeChangeData& Data);
	void HandleCoinChanged(const FOnAttributeChangeData& Data);
	void ShowPositiveAttributeDelta(float Delta, const FLinearColor& Color, const FLinearColor& OutlineColor, float ZOffset) const;

	UPROPERTY(EditDefaultsOnly, Category="Feedback")
	TSubclassOf<AMAFloatingTextActor> FloatingTextActorClass;

	TWeakObjectPtr<UMAAbilitySystemComponent> BoundASC;
	FDelegateHandle ShieldChangedHandle;
	FDelegateHandle CoinChangedHandle;
};
