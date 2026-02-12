// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_SendNewPlayerMat.h"
#include "Character/MACharacter.h"
#include "Player/Loadout/LoadoutComponent.h"

void UAnimNotifyState_SendNewPlayerMat::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                                    const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp)
	{
		AMACharacter* OwnerChar = Cast<AMACharacter>(MeshComp->GetOwner());
		if (OwnerChar && OwnerChar->HasAuthority())
		{
			if (ULoadoutComponent* LoadoutComp = OwnerChar->FindComponentByClass<ULoadoutComponent>())
			{
				LoadoutComp->SetMaterialParams(BodyParam, EyeParam);
			}
		}
	}
}

void UAnimNotifyState_SendNewPlayerMat::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		AMACharacter* OwnerChar = Cast<AMACharacter>(MeshComp->GetOwner());
		if (OwnerChar && OwnerChar->HasAuthority())
		{
			if (ULoadoutComponent* LoadoutComp = OwnerChar->FindComponentByClass<ULoadoutComponent>())
			{
				const FMaterialParamDataPair& BaseParam = LoadoutComp->GetBaseMaterialParam();
				LoadoutComp->SetMaterialParams(BaseParam.BodyData, BaseParam.EyeData);
			}
		}
	}
}
