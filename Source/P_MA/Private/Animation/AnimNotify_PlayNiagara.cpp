// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_PlayNiagara.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/MACharacter.h"

void UAnimNotify_PlayNiagara::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !MeshComp)
	{
		Super::Notify(MeshComp, Animation, EventReference);
		return;
	}
	
	if (bSpawnInWorld) // --- 월드 스폰 로직 ---
	{
#if WITH_EDITOR 
		UWorld* World = MeshComp->GetWorld();
		if (World && World->WorldType == EWorldType::EditorPreview)
		{
			FTransform SocketTransform = (SocketName != NAME_None) ? MeshComp->GetSocketTransform(SocketName) : MeshComp->GetComponentTransform();
			FTransform OffsetTransform(RotationOffset, LocationOffset, Scale);
			FTransform WorldSpawnTransform = OffsetTransform * SocketTransform;

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,VFXToSpawn,	WorldSpawnTransform.GetLocation(),WorldSpawnTransform.Rotator(),
				WorldSpawnTransform.GetScale3D(),true);

			Super::Notify(MeshComp, Animation, EventReference);
			return;
		}
#endif

		if (OwnerActor->HasAuthority())
		{
			AMACharacter* Character = Cast<AMACharacter>(OwnerActor);
			if (Character)
			{
				FTransform SocketTransform = (SocketName != NAME_None) ?
					MeshComp->GetSocketTransform(SocketName) :
					MeshComp->GetComponentTransform();

				FTransform OffsetTransform(RotationOffset, LocationOffset, Scale);
				FTransform WorldSpawnTransform = OffsetTransform * SocketTransform;

				Character->Multicast_PlayNiagara(VFXToSpawn, WorldSpawnTransform);
			}
		}
	}
	else // --- 소켓 부착 로직 ---
	{
#if WITH_EDITOR
		UWorld* World = MeshComp->GetWorld();
		if (World && World->WorldType == EWorldType::EditorPreview)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				VFXToSpawn, MeshComp, SocketName, LocationOffset, RotationOffset, Scale,
				EAttachLocation::KeepRelativeOffset, bAutoDestroy, ENCPoolMethod::None, true
			);
			
			Super::Notify(MeshComp, Animation, EventReference);
			return;
		}
#endif
		if (OwnerActor->HasAuthority())
		{
			AMACharacter* Character = Cast<AMACharacter>(OwnerActor);
			if (Character)
			{
				Character->Multicast_PlayNiagaraAttached(
					VFXToSpawn,
					SocketName,
					LocationOffset,
					RotationOffset,
					Scale,
					bAutoDestroy
				);
			}
		}
	}
	Super::Notify(MeshComp, Animation, EventReference);
}
