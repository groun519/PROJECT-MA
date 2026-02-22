// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CueEvent/MAGameplayCue_HitVFX.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UMAGameplayCue_HitVFX::UMAGameplayCue_HitVFX()
{
}

bool UMAGameplayCue_HitVFX::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (HitNiagaraVFX)
	{
		float ScaleMultiplier = Parameters.RawMagnitude > 0.f? Parameters.RawMagnitude : 1.f;
		float FinalSize = BaseVFXRadius * ScaleMultiplier;

		UNiagaraComponent* SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget, HitNiagaraVFX, Parameters.Location, Parameters.Normal.Rotation(), FVector(1.f));

		if (SpawnedNiagara)
		{
			SpawnedNiagara->SetFloatParameter(FName("User._Size"),FinalSize);
		}
	}
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
