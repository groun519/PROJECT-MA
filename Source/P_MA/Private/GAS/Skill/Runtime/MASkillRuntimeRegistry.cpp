#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"

void UMASkillRuntimeRegistry::Register(AActor* Actor)
{
	if (!Actor) return;
	if (bCleanedUp)
	{
		Actor->Destroy();
		return;
	}

	Actors.RemoveAllSwap([](const TWeakObjectPtr<AActor>& RegisteredActor)
	{
		return !RegisteredActor.IsValid();
	});
	Actors.Add(Actor);
}

void UMASkillRuntimeRegistry::Register(
	UAbilitySystemComponent* AbilitySystemComponent,
	FActiveGameplayEffectHandle Handle)
{
	if (!AbilitySystemComponent || !Handle.IsValid()) return;
	if (bCleanedUp)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(Handle, 1);
		return;
	}

	GameplayEffects.RemoveAllSwap([](const FMASkillRegisteredGameplayEffect& RegisteredEffect)
	{
		UAbilitySystemComponent* RegisteredASC = RegisteredEffect.AbilitySystemComponent.Get();
		return !RegisteredASC || !RegisteredASC->GetActiveGameplayEffect(RegisteredEffect.Handle);
	});

	FMASkillRegisteredGameplayEffect& RegisteredEffect = GameplayEffects.AddDefaulted_GetRef();
	RegisteredEffect.AbilitySystemComponent = AbilitySystemComponent;
	RegisteredEffect.Handle = Handle;
}

void UMASkillRuntimeRegistry::Cleanup()
{
	if (bCleanedUp) return;
	bCleanedUp = true;

	const TArray<TWeakObjectPtr<AActor>> RegisteredActors = MoveTemp(Actors);
	const TArray<FMASkillRegisteredGameplayEffect> RegisteredEffects = MoveTemp(GameplayEffects);

	for (const TWeakObjectPtr<AActor>& RegisteredActor : RegisteredActors)
	{
		if (AActor* Actor = RegisteredActor.Get()) Actor->Destroy();
	}

	for (const FMASkillRegisteredGameplayEffect& RegisteredEffect : RegisteredEffects)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = RegisteredEffect.AbilitySystemComponent.Get())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(RegisteredEffect.Handle, 1);
		}
	}
}
