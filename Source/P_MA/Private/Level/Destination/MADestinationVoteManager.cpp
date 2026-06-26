#include "Level/Destination/MADestinationVoteManager.h"

#include "GameFramework/PlayerState.h"
#include "Level/Destination/MADestinationVoteActor.h"
#include "Player/MAPlayerCharacter.h"

AMADestinationVoteManager::AMADestinationVoteManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AMADestinationVoteManager::BeginPlay()
{
	Super::BeginPlay();
	RegisterVoteActors();
}

void AMADestinationVoteManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterVoteActors();
	Super::EndPlay(EndPlayReason);
}

bool AMADestinationVoteManager::RequestVote(AMAPlayerCharacter* Interactor, AMADestinationVoteActor* VoteActor)
{
	if (!HasAuthority() || !Interactor || !IsManagedVoteActor(VoteActor)) return false;

	APlayerState* PlayerState = Interactor->GetPlayerState();
	if (!PlayerState) return false;

	bool bChanged = VoteActor->ApplyVote(PlayerState);
	if (!bChanged) return false;

	for (AMADestinationVoteActor* ManagedActor : VoteActors)
	{
		if (!ManagedActor || ManagedActor == VoteActor) continue;
		bChanged |= ManagedActor->RemoveVote(PlayerState);
	}

	if (bChanged)
	{
		NotifyVoteChanged();
	}
	return bChanged;
}

void AMADestinationVoteManager::ClearVotes()
{
	if (!HasAuthority()) return;

	bool bChanged = false;
	for (AMADestinationVoteActor* VoteActor : VoteActors)
	{
		if (!VoteActor) continue;
		bChanged |= VoteActor->ClearVotes();
	}

	if (bChanged)
	{
		NotifyVoteChanged();
	}
}

void AMADestinationVoteManager::RegisterVoteActors()
{
	for (AMADestinationVoteActor* VoteActor : VoteActors)
	{
		if (!VoteActor) continue;

		VoteActor->OnVoteRequested.RemoveAll(this);
		VoteActor->OnVoteRequested.AddUObject(this, &AMADestinationVoteManager::HandleVoteRequested);
	}
}

void AMADestinationVoteManager::UnregisterVoteActors()
{
	for (AMADestinationVoteActor* VoteActor : VoteActors)
	{
		if (VoteActor) VoteActor->OnVoteRequested.RemoveAll(this);
	}
}

void AMADestinationVoteManager::HandleVoteRequested(AMADestinationVoteActor* VoteActor, AMAPlayerCharacter* Interactor)
{
	RequestVote(Interactor, VoteActor);
}

bool AMADestinationVoteManager::IsManagedVoteActor(const AMADestinationVoteActor* VoteActor) const
{
	return VoteActor && VoteActors.Contains(VoteActor);
}

void AMADestinationVoteManager::NotifyVoteChanged()
{
	OnVoteChanged.Broadcast(this);
}
