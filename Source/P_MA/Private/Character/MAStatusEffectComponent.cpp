#include "Character/MAStatusEffectComponent.h"

#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	FStatusEffectRule MakeStatusEffectRule(
		const FGameplayTag& CrowdControlTag,
		const EStatusEffectImpulseMode ImpulseMode,
		const bool bStopMovementOnStart,
		const bool bPlayMontageOnStart = true,
		const bool bStopMontageOnEnd = true)
	{
		FStatusEffectRule StatusEffectRule;
		StatusEffectRule.CrowdControlTag = CrowdControlTag;
		StatusEffectRule.ImpulseMode = ImpulseMode;
		StatusEffectRule.bPlayMontageOnStart = bPlayMontageOnStart;
		StatusEffectRule.bStopMontageOnEnd = bStopMontageOnEnd;
		StatusEffectRule.bStopMovementOnStart = bStopMovementOnStart;
		return StatusEffectRule;
	}

	void AddDefaultStateStatusEffectRules(TArray<FStatusEffectRule>& StatusEffectRules)
	{
		StatusEffectRules.Add(MakeStatusEffectRule(
			UMAAbilitySystemStatics::GetAirborneStatTag(),
			EStatusEffectImpulseMode::None,
			true));

		StatusEffectRules.Add(MakeStatusEffectRule(
			UMAAbilitySystemStatics::GetStunStatTag(),
			EStatusEffectImpulseMode::None,
			true));

		StatusEffectRules.Add(MakeStatusEffectRule(
			UMAAbilitySystemStatics::GetRootStatTag(),
			EStatusEffectImpulseMode::None,
			true,
			false,
			false));
	}

	void AddDefaultImpulseStatusEffectRules(TArray<FStatusEffectRule>& StatusEffectRules)
	{
		StatusEffectRules.Add(MakeStatusEffectRule(
			UMAAbilitySystemStatics::GetGrabStatTag(),
			EStatusEffectImpulseMode::PullToSource,
			false));

		StatusEffectRules.Add(MakeStatusEffectRule(
			UMAAbilitySystemStatics::GetKnockbackStatTag(),
			EStatusEffectImpulseMode::PushFromSource,
			false));

		StatusEffectRules.Add(MakeStatusEffectRule(
			UMAAbilitySystemStatics::GetStaggerStatTag(),
			EStatusEffectImpulseMode::PushFromSource,
			true));
	}
}

UMAStatusEffectComponent::UMAStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UMAStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AMACharacter>(GetOwner());
	if (USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr)
	{
		BaseAirborneMeshRelativeLocation = MeshComponent->GetRelativeLocation();
	}
	BindToASC();
}

void UMAStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetTransientStatusEffectState();
	Super::EndPlay(EndPlayReason);
}

void UMAStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAirborneVisual(DeltaTime);
}

const FStatusEffectRule* UMAStatusEffectComponent::FindStatusEffectRule(const FGameplayTag& StatusEffectTag) const
{
	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (StatusEffectRule.CrowdControlTag == StatusEffectTag) return &StatusEffectRule;
	}
	return nullptr;
}

UMAImpulseComponent* UMAStatusEffectComponent::GetImpulseComponent() const
{
	return OwnerCharacter ? OwnerCharacter->GetImpulseComponent() : nullptr;
}

void UMAStatusEffectComponent::ApplyStatusEffectImpulse(const FStatusEffectRule& StatusEffectRule, float Magnitude, const FVector& SourcePoint)
{
	if (!StatusEffectRule.HasImpulseEffect() || Magnitude <= 0.f) return;

	if (UMAImpulseComponent* ImpulseComponent = GetImpulseComponent())
	{
		ImpulseComponent->ApplyStatusEffectImpulse(StatusEffectRule.ImpulseMode, Magnitude, SourcePoint, StatusEffectRule.CrowdControlTag);
	}
}

void UMAStatusEffectComponent::BindToASC()
{
	if (!OwnerCharacter) return;

	OwnerASC = Cast<UMAAbilitySystemComponent>(OwnerCharacter->GetAbilitySystemComponent());
	if (!OwnerASC) return;
	StatusEffectRules.Reset();
	AddDefaultStateStatusEffectRules(StatusEffectRules);
	AddDefaultImpulseStatusEffectRules(StatusEffectRules);
	if (StatusEffectRules.Num() == 0) return;

	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (!StatusEffectRule.IsValid()) continue;
		OwnerASC->RegisterGameplayTagEvent(StatusEffectRule.CrowdControlTag).AddUObject(this, &UMAStatusEffectComponent::HandleCrowdControlChanged);
	}

	OwnerASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UMAStatusEffectComponent::HandleCrowdControlApplied);
}

/*************************************************************/
/*					CrowdControl Changed					 */
/*************************************************************/
void UMAStatusEffectComponent::HandleCrowdControlChanged(FGameplayTag Tag, int32 NewCount)
{
	const FStatusEffectRule* StatusEffectRule = FindStatusEffectRule(Tag);
	if (!StatusEffectRule) return;

	if (NewCount > 0)
	{
		const bool bWasAdded = !ActiveCrowdControlTags.Contains(Tag);
		ActiveCrowdControlTags.Add(Tag);
		if (bWasAdded) HandleCrowdControlStarted(*StatusEffectRule);
		return;
	}

	if (ActiveCrowdControlTags.Remove(Tag) > 0) HandleCrowdControlEnded(*StatusEffectRule);
}

void UMAStatusEffectComponent::HandleCrowdControlStarted(const FStatusEffectRule& StatusEffectRule)
{
	if (!OwnerCharacter || OwnerCharacter->IsDead()) return;

	if (StatusEffectRule.bPlayMontageOnStart)
	{
		FStatusEffectAnimConfig StatusEffectAnimConfig;
		if (GetStatusEffectAnimConfig(StatusEffectRule.CrowdControlTag, StatusEffectAnimConfig) && StatusEffectAnimConfig.Montage)
		{
			OwnerCharacter->PlayAnimMontage(StatusEffectAnimConfig.Montage);
		}
	}

	if (StatusEffectRule.bStopMovementOnStart)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			CharacterMovementComponent->StopMovementImmediately();
		}
	}

	if (StatusEffectRule.CrowdControlTag == UMAAbilitySystemStatics::GetAirborneStatTag())
	{
		BeginAirborneVisual();
	}
}

void UMAStatusEffectComponent::HandleCrowdControlEnded(const FStatusEffectRule& StatusEffectRule)
{
	if (StatusEffectRule.bStopMontageOnEnd)
	{
		StopStatusEffectMontage(StatusEffectRule.CrowdControlTag);
	}

	if (StatusEffectRule.CrowdControlTag == UMAAbilitySystemStatics::GetAirborneStatTag())
	{
		EndAirborneVisual();
	}

	if (StatusEffectRule.HasImpulseEffect())
	{
		if (UMAImpulseComponent* ImpulseComponent = GetImpulseComponent())
		{
			ImpulseComponent->RemoveImpulse(StatusEffectRule.CrowdControlTag);
		}
	}
}

/*************************************************************/
/*					CrowdControl Applied					 */
/*************************************************************/
void UMAStatusEffectComponent::HandleCrowdControlApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle)
{
	(void)SourceASC;
	(void)ActiveHandle;

	if (!OwnerCharacter || !OwnerASC) return;

	HandleAirborneCrowdControlApplied(Spec);
	if (OwnerCharacter->HasAuthority()) HandleImpulseCrowdControlApplied(Spec);
}

void UMAStatusEffectComponent::HandleAirborneCrowdControlApplied(const FGameplayEffectSpec& Spec)
{
	const FGameplayTag AirborneTag = UMAAbilitySystemStatics::GetAirborneStatTag();
	if (!Spec.DynamicGrantedTags.HasTag(AirborneTag)) return;

	USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	if (!MeshComponent) return;

	const float AppliedHeight = Spec.GetSetByCallerMagnitude(AirborneTag, false, 0.f);
	CurrentAirborneVisualHeight = !FMath::IsNearlyZero(AppliedHeight) ? AppliedHeight : AirborneVisualHeight;
	CurrentAirborneVisualDuration = FMath::Max(Spec.GetDuration(), 0.f);

	const float AppliedRiseTime = Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetAirborneRiseTimeTag(), false, 0.f);
	if (CurrentAirborneVisualDuration > 0.f)
	{
		CurrentAirborneVisualRiseTime = AppliedRiseTime > 0.f ? FMath::Min(AppliedRiseTime, CurrentAirborneVisualDuration) : CurrentAirborneVisualDuration * 0.5f;
	}
	else
	{
		CurrentAirborneVisualRiseTime = 0.f;
	}

	AirborneVisualStartMeshRelativeLocation = bAirborneVisualActive
		? MeshComponent->GetRelativeLocation()
		: BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation.Z = FMath::Max(
		AirborneVisualStartMeshRelativeLocation.Z,
		BaseAirborneMeshRelativeLocation.Z + CurrentAirborneVisualHeight);
	CurrentAirborneVisualElapsedTime = 0.f;

	if (bAirborneVisualActive) SetComponentTickEnabled(true);
}

void UMAStatusEffectComponent::HandleImpulseCrowdControlApplied(const FGameplayEffectSpec& Spec)
{
	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (!StatusEffectRule.HasImpulseEffect()) continue;
		if (!Spec.DynamicGrantedTags.HasTag(StatusEffectRule.CrowdControlTag)) continue;

		const float Magnitude = Spec.GetSetByCallerMagnitude(StatusEffectRule.CrowdControlTag, false, 0.f);
		if (Magnitude <= 0.f) continue;

		FVector SourcePoint = FVector::ZeroVector;
		if (!UMAAbilitySystemStatics::TryGetReactionSourcePoint(Spec, SourcePoint))
		{
			if (const AActor* InstigatorActor = Spec.GetContext().GetInstigator())
			{
				SourcePoint = InstigatorActor->GetActorLocation();
			}
			else
			{
				SourcePoint = OwnerCharacter ? OwnerCharacter->GetActorLocation() : FVector::ZeroVector;
			}
		}

		ApplyStatusEffectImpulse(StatusEffectRule, Magnitude, SourcePoint);
		OwnerCharacter->Multicast_PlayStatusEffectImpulse(StatusEffectRule.CrowdControlTag, Magnitude, SourcePoint);
	}
}

void UMAStatusEffectComponent::PlayReplicatedStatusEffectImpulse(const FGameplayTag& StatusEffectTag, float Magnitude, const FVector& SourcePoint)
{
	const FStatusEffectRule* StatusEffectRule = FindStatusEffectRule(StatusEffectTag);
	if (!StatusEffectRule) return;

	ApplyStatusEffectImpulse(*StatusEffectRule, Magnitude, SourcePoint);
}

void UMAStatusEffectComponent::StopStatusEffectMontage(const FGameplayTag& StatusEffectTag)
{
	if (!OwnerCharacter) return;

	FStatusEffectAnimConfig StatusEffectAnimConfig;
	if (GetStatusEffectAnimConfig(StatusEffectTag, StatusEffectAnimConfig) && StatusEffectAnimConfig.Montage)
	{
		OwnerCharacter->StopAnimMontage(StatusEffectAnimConfig.Montage);
	}
}

void UMAStatusEffectComponent::StopAllStatusEffectMontages()
{
	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (!StatusEffectRule.IsValid()) continue;
		StopStatusEffectMontage(StatusEffectRule.CrowdControlTag);
	}
}

void UMAStatusEffectComponent::BeginAirborneVisual()
{
	if (!OwnerCharacter || bAirborneVisualActive) return;

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	if (!MeshComponent) return;

	AirborneVisualStartMeshRelativeLocation = MeshComponent->GetRelativeLocation();
	AirborneVisualPeakMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation.Z = FMath::Max(
		AirborneVisualStartMeshRelativeLocation.Z,
		BaseAirborneMeshRelativeLocation.Z + CurrentAirborneVisualHeight);
	bAirborneVisualActive = true;
	CurrentAirborneVisualElapsedTime = 0.f;
	SetComponentTickEnabled(true);
}

void UMAStatusEffectComponent::EndAirborneVisual()
{
	if (!OwnerCharacter || !bAirborneVisualActive) return;

	if (USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh()) MeshComponent->SetRelativeLocation(BaseAirborneMeshRelativeLocation);

	bAirborneVisualActive = false;
	CurrentAirborneVisualHeight = 0.f;
	CurrentAirborneVisualDuration = 0.f;
	CurrentAirborneVisualRiseTime = 0.f;
	CurrentAirborneVisualElapsedTime = 0.f;
	AirborneVisualStartMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	SetComponentTickEnabled(false);
}

void UMAStatusEffectComponent::UpdateAirborneVisual(float DeltaTime)
{
	if (!OwnerCharacter || !bAirborneVisualActive)
	{
		SetComponentTickEnabled(false);
		return;
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	if (!MeshComponent)
	{
		SetComponentTickEnabled(false);
		return;
	}

	CurrentAirborneVisualElapsedTime = FMath::Min(CurrentAirborneVisualElapsedTime + DeltaTime, CurrentAirborneVisualDuration);

	const float ClampedRiseTime = FMath::Clamp(CurrentAirborneVisualRiseTime, 0.f, CurrentAirborneVisualDuration);
	FVector NextLocation = BaseAirborneMeshRelativeLocation;

	if (CurrentAirborneVisualElapsedTime <= ClampedRiseTime && ClampedRiseTime > KINDA_SMALL_NUMBER)
	{
		const float RiseAlpha = FMath::Clamp(CurrentAirborneVisualElapsedTime / ClampedRiseTime, 0.f, 1.f);
		const float EasedRiseAlpha = 1.f - FMath::Square(1.f - RiseAlpha);
		NextLocation = FMath::Lerp(AirborneVisualStartMeshRelativeLocation, AirborneVisualPeakMeshRelativeLocation, EasedRiseAlpha);
	}
	else if (CurrentAirborneVisualDuration > KINDA_SMALL_NUMBER)
	{
		const float FallDuration = FMath::Max(CurrentAirborneVisualDuration - ClampedRiseTime, KINDA_SMALL_NUMBER);
		const float FallAlpha = FMath::Clamp((CurrentAirborneVisualElapsedTime - ClampedRiseTime) / FallDuration, 0.f, 1.f);
		const float EasedFallAlpha = FMath::Square(FallAlpha);
		NextLocation = FMath::Lerp(AirborneVisualPeakMeshRelativeLocation, BaseAirborneMeshRelativeLocation, EasedFallAlpha);
	}

	MeshComponent->SetRelativeLocation(NextLocation);

	if (CurrentAirborneVisualElapsedTime >= CurrentAirborneVisualDuration)
	{
		MeshComponent->SetRelativeLocation(BaseAirborneMeshRelativeLocation);
		SetComponentTickEnabled(false);
	}
}

bool UMAStatusEffectComponent::GetStatusEffectAnimConfig(const FGameplayTag& StatusEffectTag, FStatusEffectAnimConfig& OutConfig) const
{
	if (const FStatusEffectAnimConfig* FoundConfig = StatusEffectAnimMap.Find(StatusEffectTag))
	{
		OutConfig = *FoundConfig;
		return true;
	}

	return false;
}

void UMAStatusEffectComponent::ResetTransientStatusEffectState()
{
	StopAllStatusEffectMontages();
	EndAirborneVisual();
	if (UMAImpulseComponent* ImpulseComponent = GetImpulseComponent())
	{
		ImpulseComponent->ResetImpulseState();
	}
	ActiveCrowdControlTags.Reset();
}
