// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SendNewPlayerTrans.h"
#include "GameFramework/Character.h"

void UAnimNotify_SendNewPlayerTrans::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		ACharacter* OwnerChar = Cast<ACharacter>(MeshComp->GetOwner());
		if (OwnerChar && OwnerChar->HasAuthority())
		{
			if (MoveType == EMoveType::None) return;
			else if (MoveType == EMoveType::Launch)
			{
				FVector Forward = OwnerChar->GetActorForwardVector();
				OwnerChar->LaunchCharacter(Forward * LaunchPower, false, false);
			}
			else if (MoveType == EMoveType::Teleport)
			{
				FRotator LookRot = OwnerChar->GetActorRotation();
				if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
				{
					FHitResult HitResult;
					PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
					if (HitResult.bBlockingHit)
					{
						FVector MouseWorldLocation = HitResult.ImpactPoint;
						OwnerChar->TeleportTo(MouseWorldLocation, LookRot, false);
					}
				}
			}
		}
	}
}
