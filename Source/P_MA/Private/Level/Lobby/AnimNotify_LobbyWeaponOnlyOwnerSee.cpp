#include "AnimNotify_LobbyWeaponOnlyOwnerSee.h"
#include "LobbyAvatarSlot.h"

void UAnimNotify_LobbyWeaponOnlyOwnerSee::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (ALobbyAvatarSlot* Slot = Cast<ALobbyAvatarSlot>(MeshComp->GetOwner()))
	{
		Slot->SetWeaponOnlyOwnerSee(bOnlyOwnerSee);
	}
}
