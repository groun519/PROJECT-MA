#include "GAS/Projectile/MANiagaraProjectile.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AMANiagaraProjectile::AMANiagaraProjectile()
{
	Niagara = CreateDefaultSubobject<UNiagaraComponent>("Niagara");
	Niagara->SetupAttachment(SphereComp);
}

void AMANiagaraProjectile::BeginPlay()
{
	Super::BeginPlay();
	ApplyProjectileVisuals();
}

void AMANiagaraProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMANiagaraProjectile, Rep_MainVFX);
}

void AMANiagaraProjectile::InitializeProjectileVisuals(const FMAProjectileParams& InProjectileParams)
{
	Rep_MainVFX = InProjectileParams.ElementalSettings.MainVFX;
	ApplyProjectileVisuals();
}

void AMANiagaraProjectile::OnProjectileRadiusChanged()
{
	ApplyProjectileVisuals();
}

void AMANiagaraProjectile::OnProjectileElementalColorChanged()
{
	ApplyProjectileVisuals();
}

void AMANiagaraProjectile::OnProjectilePendingDestroy()
{
	if (!Niagara) return;

	Niagara->Deactivate();
	Niagara->SetVisibility(false, true);
}

void AMANiagaraProjectile::ApplyProjectileVisuals()
{
	if (HasProjectileElementalData() && MainVisualSettings.bUseElementalVFX && Rep_MainVFX
		&& Niagara->GetAsset() != Rep_MainVFX)
	{
		Niagara->SetAsset(Rep_MainVFX);
		Niagara->ResetSystem();
	}

	if (HasProjectileElementalData() && MainVisualSettings.bUseElementalColor)
	{
		Niagara->SetVariableLinearColor(TEXT("User.BaseColor"), GetProjectileElementalColor());
	}

	Niagara->SetVariableFloat(TEXT("User.Radius"), SphereComp->GetScaledSphereRadius());
}
