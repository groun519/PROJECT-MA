// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_PlayNiagara.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/MACharacter.h"
#include "Player/MAPlayerCharacter.h"

void UAnimNotify_PlayNiagara::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp || !NiagaraTemplate)
		return;

#if WITH_EDITOR
	UWorld* World = MeshComp->GetWorld();
	if (World && World->WorldType == EWorldType::EditorPreview)
	{
		if (bSpawnInWorld)
		{
			FTransform SocketTransform = (SocketName != NAME_None) ? MeshComp->GetSocketTransform(SocketName) : MeshComp->GetComponentTransform();
			FTransform OffsetTransform(RotationOffset, LocationOffset, Scale);
			FTransform WorldSpawnTransform = OffsetTransform * SocketTransform;

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NiagaraTemplate,
				WorldSpawnTransform.GetLocation(), WorldSpawnTransform.Rotator(), WorldSpawnTransform.GetScale3D());
		}
		else
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraTemplate, MeshComp, SocketName, 
				LocationOffset, RotationOffset, Scale, EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true);
		}
		return;
	}
#endif
	AMACharacter* Character = Cast<AMACharacter>(MeshComp->GetOwner());
	
	UNiagaraSystem* FinalVFXToSpawn = NiagaraTemplate;
	FLinearColor ModuleColor = FLinearColor::White;
	FVector FinalScale = Scale;

	if (AMAPlayerCharacter* PlayerChar = Cast<AMAPlayerCharacter>(Character))
	{
		if (!PlayerChar->GetAllowVFX())
			return;
		
		ModuleColor = PlayerChar->GetCurrentVFXColor();
		FGameplayTag CurrentTag = PlayerChar->GetCurrentElementTag();
		
		if (CurrentTag.IsValid() && OverrideVFXMap.Contains(CurrentTag))
		{
			FinalVFXToSpawn = OverrideVFXMap[CurrentTag];
		}

		if (BaseVFXLength > 0.f)
		{
			float TargetVFXLength = PlayerChar->GetCurrentVFXLength();
			if (TargetVFXLength > 0.f)
			{
				float ScaleMultiplier = TargetVFXLength / BaseVFXLength;
				FinalScale.X *= ScaleMultiplier;
			}
		}
		if (!FinalVFXToSpawn)
			return;
	}
		
	UNiagaraComponent* SpawnedVFX = nullptr;
	if (bSpawnInWorld)
	{
		FTransform SocketTransform = (SocketName != NAME_None) ? MeshComp->GetSocketTransform(SocketName) : MeshComp->GetComponentTransform();
		FTransform OffsetTransform(RotationOffset, LocationOffset, FinalScale);
		FTransform WorldSpawnTransform = OffsetTransform * SocketTransform;

		SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MeshComp->GetWorld(), FinalVFXToSpawn,
			WorldSpawnTransform.GetLocation(), WorldSpawnTransform.Rotator(), WorldSpawnTransform.GetScale3D());
	}
	else
	{
		SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(FinalVFXToSpawn, MeshComp, SocketName, 
			LocationOffset, RotationOffset, FinalScale,
			EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true);
	}

	if (SpawnedVFX && ColorParamName!=NAME_None)
	{
		SpawnedVFX->SetColorParameter(ColorParamName, ModuleColor);
	}
}
