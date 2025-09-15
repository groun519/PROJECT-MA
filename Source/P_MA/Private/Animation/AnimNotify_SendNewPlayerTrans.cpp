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
			/** MoveType:None **/
			if (MoveType == EMoveType::None) return;

			/** MoveType:Launch **//**
			 *	- 플레이어 캐릭터를 LaunchCharacter를 이용해 '발사'한다.
			 */
			else if (MoveType == EMoveType::Launch)
			{
				FVector Forward = OwnerChar->GetActorForwardVector();
				OwnerChar->LaunchCharacter(Forward * LaunchPower, true, true);
			}

			/** MoveType:Teleport **//**
			 *	- 플레이어 캐릭터의 Location Teleport를 이용해
			 */
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
