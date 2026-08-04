#include "Animation/Notify/AnimNotifyState_HideWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponComponent.h"

static void SetOwnerWeaponVisible(const USkeletalMeshComponent* MeshComp, bool bVisible)
{
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	UWeaponComponent* WeaponComponent = Owner ? Owner->FindComponentByClass<UWeaponComponent>() : nullptr;
	if (WeaponComponent) WeaponComponent->SetVisibility(bVisible, true);
}

void UAnimNotifyState_HideWeapon::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SetOwnerWeaponVisible(MeshComp, false);
}

void UAnimNotifyState_HideWeapon::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SetOwnerWeaponVisible(MeshComp, true);
}
