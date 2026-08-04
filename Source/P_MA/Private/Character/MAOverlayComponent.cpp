#include "Character/MAOverlayComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"

UMAOverlayComponent::UMAOverlayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMAOverlayComponent::BeginPlay()
{
	Super::BeginPlay();

	TargetMesh = ResolveTargetMesh();
}

UMaterialInstanceDynamic* UMAOverlayComponent::GetOrCreateOverlay()
{
	if (OverlayMID) return OverlayMID;

	if (!TargetMesh)
	{
		TargetMesh = ResolveTargetMesh();
	}
	if (!TargetMesh) return nullptr;

	UMaterialInterface* Material = UMAGameSettings::Get()->GetOverlayMaterial();
	check(Material);
	OverlayMID = UMaterialInstanceDynamic::Create(Material, this);
	check(OverlayMID);
	OverlayMID->SetScalarParameterValue(PARAM_Overlay_Alpha, 0.f);
	OverlayMID->SetVectorParameterValue(PARAM_Overlay_BaseColor, FLinearColor::White);
	OverlayMID->SetScalarParameterValue(PARAM_Overlay_TemperatureAlpha, 0.5f);
	TargetMesh->SetOverlayMaterial(OverlayMID);
	return OverlayMID;
}

void UMAOverlayComponent::ApplyTimedOverlay(const FLinearColor& BaseColor, float Alpha, float Duration)
{
	if (Duration <= 0.f) return;

	UMaterialInstanceDynamic* MID = GetOrCreateOverlay();
	if (!MID) return;

	MID->SetVectorParameterValue(PARAM_Overlay_BaseColor, BaseColor);
	MID->SetScalarParameterValue(PARAM_Overlay_Alpha, FMath::Clamp(Alpha, 0.f, 1.f));

	GetWorld()->GetTimerManager().SetTimer(
		TimedOverlayTimerHandle,
		this,
		&UMAOverlayComponent::ClearTimedOverlay,
		Duration,
		false);
}

void UMAOverlayComponent::ClearTimedOverlay()
{
	OverlayMID->SetScalarParameterValue(PARAM_Overlay_Alpha, 0.f);
}

USkeletalMeshComponent* UMAOverlayComponent::ResolveTargetMesh() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		return Character->GetMesh();
	}

	return GetOwner() ? GetOwner()->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
}
