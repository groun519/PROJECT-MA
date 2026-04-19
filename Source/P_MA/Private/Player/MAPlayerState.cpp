#include "MAPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Framework/MAGameInstance.h"

// NOTE:
// Seamless travel 과정에서 새 PlayerState 인스턴스로 교체될 때
// 로드아웃/슬롯 정보가 기본값으로 돌아가지 않게 수동 복사한다.
void AMAPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AMAPlayerState* NewPS = Cast<AMAPlayerState>(PlayerState);
	if (!NewPS) return;

	NewPS->DefaultSkill = DefaultSkill;
	NewPS->LoadoutSelection = LoadoutSelection;
	NewPS->bHasFinishedLoading = bHasFinishedLoading;
	NewPS->LobbySlotIndex = LobbySlotIndex;
}

void AMAPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	const AMAPlayerState* OldPS = Cast<AMAPlayerState>(PlayerState);
	if (!OldPS) return;

	DefaultSkill = OldPS->DefaultSkill;
	LoadoutSelection = OldPS->LoadoutSelection;
	bHasFinishedLoading = OldPS->bHasFinishedLoading;
	LobbySlotIndex = OldPS->LobbySlotIndex;
}

void AMAPlayerState::SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill)
{
	DefaultSkill = NewSkill;
}

void AMAPlayerState::SetLoadoutSelection(const FLoadoutSelection& NewLoadout)
{
	LoadoutSelection = NewLoadout;
	OnLoadoutChanged.Broadcast(LoadoutSelection);
}

const FLoadoutSelection& AMAPlayerState::GetLoadoutSelection() const
{
	return LoadoutSelection;
}

void AMAPlayerState::SetLoadingComplete(bool bComplete)
{
	bHasFinishedLoading = bComplete;
}

void AMAPlayerState::SetLobbySlotIndex(int32 Index)
{
	LobbySlotIndex = Index;
}

void AMAPlayerState::OnRep_DefaultSkill()
{
}

void AMAPlayerState::OnRep_LoadoutSelection()
{
	OnLoadoutChanged.Broadcast(LoadoutSelection);
}

void AMAPlayerState::OnRep_LoadingComplete()
{
	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		GI->UpdateLoadingStatus();
	}
}

void AMAPlayerState::OnRep_LobbySlotIndex()
{
}

void AMAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMAPlayerState, DefaultSkill);
	DOREPLIFETIME(AMAPlayerState, LoadoutSelection);
	DOREPLIFETIME(AMAPlayerState, bHasFinishedLoading);
	DOREPLIFETIME(AMAPlayerState, LobbySlotIndex);
}
