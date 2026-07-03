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
		const FGameplayTag& StatusEffectTag,
		const EStatusEffectImpulseMode ImpulseMode,
		const bool bStopMovementOnStart,
		const bool bPlayMontageOnStart = true,
		const bool bStopMontageOnEnd = true)
	{
		FStatusEffectRule StatusEffectRule;
		StatusEffectRule.StatusEffectTag = StatusEffectTag;
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
			UMAAbilitySystemStatics::GetFrozenStatTag(),
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
	SetIsReplicatedByDefault(true);
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
		if (StatusEffectRule.StatusEffectTag == StatusEffectTag) return &StatusEffectRule;
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
		ImpulseComponent->ApplyStatusEffectImpulse(StatusEffectRule.ImpulseMode, Magnitude, SourcePoint, StatusEffectRule.StatusEffectTag);
	}
}

FText UMAStatusEffectComponent::MakeStatusEffectDisplayLabel(const FGameplayTag& StatusEffectTag) const
{
	const FString TagString = StatusEffectTag.GetTagName().ToString();
	FString LabelString;
	if (!TagString.Split(TEXT("."), nullptr, &LabelString, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		LabelString = TagString;
	}

	return FText::FromString(LabelString);
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
		OwnerASC->RegisterGameplayTagEvent(StatusEffectRule.StatusEffectTag, EGameplayTagEventType::AnyCountChange).AddUObject(this, &UMAStatusEffectComponent::HandleStatusEffectChanged);
	}

	OwnerASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UMAStatusEffectComponent::HandleStatusEffectApplied);
}

/*************************************************************/
/*					StatusEffect Changed					 */
/*************************************************************/
void UMAStatusEffectComponent::HandleStatusEffectChanged(FGameplayTag Tag, int32 NewCount)
{
	const FStatusEffectRule* StatusEffectRule = FindStatusEffectRule(Tag);
	if (!StatusEffectRule) return;

	if (NewCount > 0)
	{
		const bool bWasAdded = !ActiveStatusEffectTags.Contains(Tag);
		ActiveStatusEffectTags.Add(Tag);
		if (bWasAdded) HandleStatusEffectStarted(*StatusEffectRule);
		return;
	}

	if (ActiveStatusEffectTags.Remove(Tag) > 0) HandleStatusEffectEnded(*StatusEffectRule);
}

void UMAStatusEffectComponent::HandleStatusEffectStarted(const FStatusEffectRule& StatusEffectRule)
{
	if (!OwnerCharacter || OwnerCharacter->IsDead()) return;

	if (StatusEffectRule.bPlayMontageOnStart)
	{
		FStatusEffectAnimConfig StatusEffectAnimConfig;
		if (GetStatusEffectAnimConfig(StatusEffectRule.StatusEffectTag, StatusEffectAnimConfig) && StatusEffectAnimConfig.Montage)
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

	if (StatusEffectRule.StatusEffectTag == UMAAbilitySystemStatics::GetAirborneStatTag())
	{
		BeginAirborneVisual();
	}
}

void UMAStatusEffectComponent::HandleStatusEffectEnded(const FStatusEffectRule& StatusEffectRule)
{
	RemoveStatusEffectDisplayState(StatusEffectRule.StatusEffectTag);

	if (StatusEffectRule.bStopMontageOnEnd)
	{
		StopStatusEffectMontage(StatusEffectRule.StatusEffectTag);
	}

	if (StatusEffectRule.StatusEffectTag == UMAAbilitySystemStatics::GetAirborneStatTag())
	{
		EndAirborneVisual();
	}

	if (StatusEffectRule.HasImpulseEffect())
	{
		if (UMAImpulseComponent* ImpulseComponent = GetImpulseComponent())
		{
			ImpulseComponent->RemoveImpulse(StatusEffectRule.StatusEffectTag);
		}
	}
}

/*************************************************************/
/*					StatusEffect Applied					 */
/*************************************************************/
void UMAStatusEffectComponent::HandleStatusEffectApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle)
{
	(void)SourceASC;
	(void)ActiveHandle;

	if (!OwnerCharacter || !OwnerASC) return;

	UpdateStatusEffectDisplayState(Spec);
	HandleAirborneStatusEffectApplied(Spec);
	if (OwnerCharacter->HasAuthority()) HandleImpulseStatusEffectApplied(Spec);
}

void UMAStatusEffectComponent::HandleAirborneStatusEffectApplied(const FGameplayEffectSpec& Spec)
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

void UMAStatusEffectComponent::HandleImpulseStatusEffectApplied(const FGameplayEffectSpec& Spec)
{
	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (!StatusEffectRule.HasImpulseEffect()) continue;
		if (!Spec.DynamicGrantedTags.HasTag(StatusEffectRule.StatusEffectTag)) continue;

		const float Magnitude = Spec.GetSetByCallerMagnitude(StatusEffectRule.StatusEffectTag, false, 0.f);
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
		Multicast_PlayStatusEffectImpulse(StatusEffectRule.StatusEffectTag, Magnitude, SourcePoint);
	}
}

void UMAStatusEffectComponent::Multicast_PlayStatusEffectImpulse_Implementation(
	const FGameplayTag& StatusEffectTag,
	float Magnitude,
	FVector SourcePoint)
{
	if (GetOwnerRole() == ROLE_Authority) return;

	PlayReplicatedStatusEffectImpulse(StatusEffectTag, Magnitude, SourcePoint);
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
		StopStatusEffectMontage(StatusEffectRule.StatusEffectTag);
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

void UMAStatusEffectComponent::UpdateStatusEffectDisplayState(const FGameplayEffectSpec& Spec)
{
	const float Duration = Spec.GetDuration();
	if (Duration <= 0.f)
	{
		return;
	}

	const double WorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	const double EndTimeSeconds = WorldTimeSeconds + Duration;

	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (!StatusEffectRule.IsValid() || !Spec.DynamicGrantedTags.HasTag(StatusEffectRule.StatusEffectTag))
		{
			continue;
		}

		FStatusEffectDisplayState& DisplayState = StatusEffectDisplayStates.FindOrAdd(StatusEffectRule.StatusEffectTag);
		DisplayState.Label = MakeStatusEffectDisplayLabel(StatusEffectRule.StatusEffectTag);
		DisplayState.Duration = Duration;
		DisplayState.EndTimeSeconds = EndTimeSeconds;
	}

	OnStatusEffectDisplayChanged.Broadcast();
}

void UMAStatusEffectComponent::RemoveStatusEffectDisplayState(const FGameplayTag& StatusEffectTag)
{
	if (StatusEffectDisplayStates.Remove(StatusEffectTag) > 0)
	{
		OnStatusEffectDisplayChanged.Broadcast();
	}
}

void UMAStatusEffectComponent::GetActiveStatusEffectDisplayEvents(TArray<FStatusEffectDisplayEvent>& OutEvents) const
{
	OutEvents.Reset();

	const double WorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	for (const FStatusEffectRule& StatusEffectRule : StatusEffectRules)
	{
		if (!ActiveStatusEffectTags.Contains(StatusEffectRule.StatusEffectTag))
		{
			continue;
		}

		const FStatusEffectDisplayState* DisplayState = StatusEffectDisplayStates.Find(StatusEffectRule.StatusEffectTag);
		if (!DisplayState)
		{
			continue;
		}

		const float RemainingDuration = FMath::Max(static_cast<float>(DisplayState->EndTimeSeconds - WorldTimeSeconds), 0.f);
		if (RemainingDuration <= 0.f || DisplayState->Duration <= 0.f)
		{
			continue;
		}

		FStatusEffectDisplayEvent& EventData = OutEvents.AddDefaulted_GetRef();
		EventData.StatusEffectTag = StatusEffectRule.StatusEffectTag;
		EventData.Label = DisplayState->Label;
		EventData.Duration = DisplayState->Duration;
		EventData.RemainingDuration = RemainingDuration;
	}

}

void UMAStatusEffectComponent::ResetTransientStatusEffectState()
{
	StopAllStatusEffectMontages();
	EndAirborneVisual();
	if (UMAImpulseComponent* ImpulseComponent = GetImpulseComponent())
	{
		ImpulseComponent->ResetImpulseState();
	}
	ActiveStatusEffectTags.Reset();
	StatusEffectDisplayStates.Reset();
	OnStatusEffectDisplayChanged.Broadcast();
}
