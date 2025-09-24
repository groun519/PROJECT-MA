// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SendNewPlayerTrans.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"

void UAnimNotify_SendNewPlayerTrans::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
		if (Owner && Owner->HasAuthority())
		{
			/** MoveType:None **/
			if (MoveType == EMoveType::None) return;
			
			float SectionStart = 0.f, SectionEnd = 0.f, PlayRate = 0.f, RateScale = 0.f;
			if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
			{
				UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
				PlayRate	= AnimInst->Montage_GetPlayRate(Montage);
				RateScale	= Montage->RateScale;
				if (int32 SectionIndex = Montage->GetSectionIndex(MoveSectionName))
				{
					if (SectionIndex != INDEX_NONE)
					{
						Montage->GetSectionStartAndEndTime(SectionIndex, SectionStart, SectionEnd);
					}
				}
			}
			
			/** MoveType:Jump **//**
			 *	- 플레이어 캐릭터를 LaunchCharacter를 이용해 '발사'한다.
			 */
			if (MoveType == EMoveType::Jump)
			{
				FGameplayEventData Data;
				{
					/**
					 *	"Jump"
					 *	Jump 타입은 특정 좌표를 목표로 이동하는 이동기입니다.
					 *	* 특정 좌표 : 특정 좌표는,
					 *				"스킬 사거리 이내"라면 마우스 커서 위치를,
					 *				"스킬 사거리 밖"이라면 마우스 커서 방향의 사거리 이내 제일 먼 로케이션을 받아와야 합니다.
					 *	특정 좌표를 UMovementAbility 클래스에서 ActivateAbility() 시에 받아오도록 하고,
					 *	필요하다면, 전송되는 이벤트에 포함된 데이터
					 *	@OwnerLocation - 캐릭터의 노티파이 시점의 로케이션
					 *	@OwnerRotation - 캐릭터의 노티파이 시점의 로테이션
					 *	을 받아와 사용할 수 있습니다.
					 *	* 받는 방법은 MAGameplayAbility 참고할 것.
					 */
					auto* JumpData = new FJumpData();
					JumpData->OwnerLocation		= Owner->GetActorLocation();
					JumpData->OwnerRotation		= Owner->GetActorRotation();
					JumpData->StartToEndTime	= (SectionEnd - SectionStart) / PlayRate / RateScale;
					JumpData->JumpTimeRequired	= JumpTimeRequired;
					Data.TargetData.Add(JumpData);
				}
				if (TagType != EMovementNotifyTags::None)
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GetJumpTag(), Data);
			}
			
			/** MoveType:Dash **//**
			 *	-
			 */
			else if (MoveType == EMoveType::Dash)
			{
				FGameplayEventData Data;
				{
					/**
					 *	"Dash"
					 *	Dash 타입은 특정 방향을 목표로 이동하는 이동기입니다.
					 *	* 특정 방향 : 특정 방향은,
					 *				스킬 시작 시점은, 단순히
					 *				(카메라 - 마우스 - Hitpoint 지점의 좌표) - (Owner의 좌표)
					 *				를 계산해 나온 방향벡터를 사용합니다.
					 *				@DashForce가 곱해지기 때문에, 정규화 된 값(Normalize())이어야 합니다.
					 *	필요하다면, 전송되는 이벤트에 포함된 데이터
					 *	@OwnerLocation - 캐릭터의 노티파이 시점의 로케이션
					 *	@OwnerRotation - 캐릭터의 노티파이 시점의 로테이션
					 *	@DashForce
					 */
					auto* DashData = new FDashData();
					DashData->OwnerLocation = Owner->GetActorLocation();
					DashData->OwnerRotation = Owner->GetActorRotation();
					DashData->DashForce = DashForce;
					Data.TargetData.Add(DashData);
				}
				if (TagType != EMovementNotifyTags::None)
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GetDashTag(), Data);
			}

			/** MoveType:Rush **//**
			 *	-
			 */
			else if (MoveType == EMoveType::Rush)
			{
				FGameplayEventData Data;
				{
					/**
					 *	"Rush"
					 *	Rush 타입은 특정 방향으로 지속적으로 이동하는 이동기입니다.
					 *	특히, 이동하며, 마우스 방향으로 회전이 가능한 이동기입니다.
					 *	* 이동하며 회전 : 
					 */
					auto* DashData = new FRushData();
					DashData->OwnerLocation = Owner->GetActorLocation();
					DashData->OwnerRotation = Owner->GetActorRotation();
					DashData->RushForce = RushForce;
					DashData->MaxRotateAngle = MaxRotateAngle;
					Data.TargetData.Add(DashData);
				}
				if (TagType != EMovementNotifyTags::None)
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GetDashTag(), Data);
			}
			
			/** MoveType:Teleport **//**
			 *	- 
			 */
			else if (MoveType == EMoveType::Teleport)
			{
				FGameplayEventData Data;
				{
					/**
					 *	"Teleport"
					 *	Teleport 타입은 특정 위치로 로케이션, 로테이션을 변경시키는 이동기입니다.
					 *	특히, 이동 시 마우스 방향을 바라보게 하는 디테일이 중요합니다.
					 *	* 로케이션 변경 : 로케이션 변경은 Jump처럼 받아온 마우스 로케이션 기반으로 TeleportTo()나
					 *					RPC 처리가 된 SetActorLocation() or SetActorTransform() 으로 구현하면 됩니다.
					 *	* 로테이션 변경 : 로테이션 변경은 Dash처럼 받아온 마우스 - Owner 위치 기반으로 구해진 방향을,
					 *					캐릭터가 이동 시 바라보게 SetActorRotation() or SetActorTransform() 해주면 됩니다.
					 *	* TeleportTo() : 언리얼에서 제공하는 함수 TeleportTo()는 이동 위치, 방향 바라보기, RPC 모두 처리해주니,
					 *					 TeleportTo()를 사용해 날먹하도록 합시다
					 */
					auto* TeleportData = new FTeleportData();
					TeleportData->OwnerLocation = Owner->GetActorLocation();
					TeleportData->OwnerRotation = Owner->GetActorRotation();
					Data.TargetData.Add(TeleportData);
				}
				if (TagType != EMovementNotifyTags::None)
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GetDashTag(), Data);
			}
		}
	}
}

/**
 * GetJumpType()
 * "Ability.Movement.Jump" 관련 태그를 반환합니다.
 * 
 * @return TagType == None	-> "Ability.Movement.Jump"
 *					  Start -> "Ability.Movement.Jump.Start"
 *					  End	-> "Ability.Movement.Jump.End"
 *
 *	bDamageTag는 노티파이에서 직접 할당받습니다.
 */
FGameplayTag UAnimNotify_SendNewPlayerTrans::GetJumpTag()
{
	switch (TagType)
	{
	case EMovementNotifyTags::None:
       	return FGameplayTag::RequestGameplayTag("Ability.Movement.Jump");
    case EMovementNotifyTags::Start:
        return FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start");
    case EMovementNotifyTags::End:
       	return FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.End");
    default:
       	return FGameplayTag();
    }	
}

/**
 * GetDashType()
 * "Ability.Movement.Dash" 관련 태그를 반환합니다.
 * 
 * @return TagType == None	-> "Ability.Movement.Dash"
 *					  Start -> "Ability.Movement.Dash.Start"
 *					  End	-> "Ability.Movement.Dash.End"
 *
 *	TagType은 노티파이에서 직접 할당받습니다.
 */
FGameplayTag UAnimNotify_SendNewPlayerTrans::GetDashTag()
{
	switch (TagType)
	{
	case EMovementNotifyTags::None:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Dash");
	case EMovementNotifyTags::Start:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Dash.Start");
	case EMovementNotifyTags::End:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Dash.End");
	default:
		return FGameplayTag();
	}
}

/**
 * GetRushType()
 * "Ability.Movement.Rush" 관련 태그를 반환합니다.
 * 
 * @return TagType == None	-> "Ability.Movement.Rush"
 *					  Start -> "Ability.Movement.Rush.Start"
 *					  End	-> "Ability.Movement.Rush.End"
 */
FGameplayTag UAnimNotify_SendNewPlayerTrans::GetRushTag()
{
	switch (TagType)
	{
	case EMovementNotifyTags::None:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Rush");
	case EMovementNotifyTags::Start:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Rush.Start");
	case EMovementNotifyTags::End:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Rush.End");
	default:
		return FGameplayTag();
	}
}

/**
 * GetTeleportType()
 * "Ability.Movement.Teleport" 관련 태그를 반환합니다.
 * 
 * @return bDamageTag == true -> "Ability.Movement.Teleport.Damage"
 *						 false-> "Ability.Movement.Teleport"
 */
FGameplayTag UAnimNotify_SendNewPlayerTrans::GetTeleportTag()
{
	switch (TagType)
	{
	case EMovementNotifyTags::None:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Teleport");
	case EMovementNotifyTags::Start:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Teleport.Start");
	case EMovementNotifyTags::End:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Teleport.End");
	default:
		return FGameplayTag();
	}
}
