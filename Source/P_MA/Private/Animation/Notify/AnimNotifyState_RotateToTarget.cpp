// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notify/AnimNotifyState_RotateToTarget.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotifyState_RotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	AActor* Target = UGameplayStatics::GetPlayerPawn(Owner, 0);
	if (!Target) return;

	// 방향 계산
	FVector Dir = Target->GetActorLocation() - Owner->GetActorLocation();
	Dir.Z = 0.f;
	if (Dir.SizeSquared() < KINDA_SMALL_NUMBER) return;

	FRotator NewRot = Dir.Rotation();
	
	if (Owner->HasAuthority())
	{
		Owner->SetActorRotation(NewRot);
	}
}
