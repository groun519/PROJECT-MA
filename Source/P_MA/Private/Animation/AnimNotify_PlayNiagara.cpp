// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_PlayNiagara.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/MACharacter.h"

void UAnimNotify_PlayNiagara::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!NiagaraSystem || !MeshComp)
	{
		Super::Notify(MeshComp, Animation, EventReference);
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		Super::Notify(MeshComp, Animation, EventReference);
		return;
	}
	
	if (bSpawnInWorld) // --- 월드 스폰 로직 ---
	{
#if WITH_EDITOR 
		UWorld* World = MeshComp->GetWorld();
		// 현재 월드가 에디터 미리보기 월드인지 확인
		if (World && World->WorldType == EWorldType::EditorPreview)
		{
			FTransform SocketTransform = (SocketName != NAME_None) ? MeshComp->GetSocketTransform(SocketName) : MeshComp->GetComponentTransform();
			FTransform OffsetTransform(RotationOffset, LocationOffset, Scale);
			FTransform WorldSpawnTransform = OffsetTransform * SocketTransform;

			// 미리보기 월드 컨텍스트를 사용하여 스폰
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,NiagaraSystem,	WorldSpawnTransform.GetLocation(),WorldSpawnTransform.Rotator(),
				WorldSpawnTransform.GetScale3D(),true);

			Super::Notify(MeshComp, Animation, EventReference); // 부모 함수 호출 잊지 않기
			return; // 아래 게임 로직 실행 안 함
		}
#endif // WITH_EDITOR

		if (OwnerActor->HasAuthority() || OwnerActor->GetLocalRole() == ROLE_AutonomousProxy)
		{
			AMACharacter* Character = Cast<AMACharacter>(OwnerActor);
			if (Character)
			{
				// 소켓 트랜스폼 가져오기
				FTransform SocketTransform = (SocketName != NAME_None) ?
					MeshComp->GetSocketTransform(SocketName) :
					MeshComp->GetComponentTransform();
				
				// 오프셋 적용
				FTransform OffsetTransform(RotationOffset, LocationOffset, Scale);
				FTransform WorldSpawnTransform = OffsetTransform * SocketTransform; // 오프셋을 먼저 적용하고 월드 트랜스폼 곱하기

				// 캐릭터의 멀티캐스트 RPC 호출
				Character->Multicast_PlayNiagara(NiagaraSystem, WorldSpawnTransform);
				// 참고: RPC 함수가 회전/크기도 받는다면 WorldSpawnTransform 전체 전달
			}
		}
	}
	else // --- 소켓 부착 로직 ---
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystem,      // System Template
			MeshComp,           // Attach to Component
			SocketName,         // Attach Point Name
			LocationOffset,     // Location
			RotationOffset,     // Rotation
			Scale,              // Scale
			EAttachLocation::KeepRelativeOffset, // Location Type
			bAutoDestroy,       // Auto Destroy
			ENCPoolMethod::None, // Pooling Method (필요시 변경)
			true                // Auto Activate
		);

	}
	Super::Notify(MeshComp, Animation, EventReference);
}
