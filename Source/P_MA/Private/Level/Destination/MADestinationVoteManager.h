#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MADestinationVoteManager.generated.h"

class AMADestinationVoteActor;
class AMAPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMADestinationVoteManagerChangedSignature, AMADestinationVoteManager*, VoteManager);

UCLASS()
class P_MA_API AMADestinationVoteManager : public AActor
{
	GENERATED_BODY()

public:
	AMADestinationVoteManager();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool RequestVote(AMAPlayerCharacter* Interactor, AMADestinationVoteActor* VoteActor);
	void ClearVotes();

	const TArray<TObjectPtr<AMADestinationVoteActor>>& GetVoteActors() const { return VoteActors; }

	UPROPERTY(BlueprintAssignable, Category="Destination|Vote")
	FMADestinationVoteManagerChangedSignature OnVoteChanged;

protected:
	UPROPERTY(EditInstanceOnly, Category="Destination|Vote")
	TArray<TObjectPtr<AMADestinationVoteActor>> VoteActors;

private:
	void RegisterVoteActors();
	void UnregisterVoteActors();
	void HandleVoteRequested(AMADestinationVoteActor* VoteActor, AMAPlayerCharacter* Interactor);
	bool IsManagedVoteActor(const AMADestinationVoteActor* VoteActor) const;
	void NotifyVoteChanged();
};
