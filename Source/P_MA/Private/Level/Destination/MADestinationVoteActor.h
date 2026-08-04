#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "MADestinationVoteActor.generated.h"

class AMAPlayerCharacter;
class APlayerState;
class UMAHighlightComponent;
class UMAInteractableComponent;
class UStaticMeshComponent;
class UTexture2D;
class UWidgetComponent;

USTRUCT(BlueprintType)
struct FMADestinationOptionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destination")
	FGameplayTag EnvTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destination")
	int32 CoinReward = 0;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FMADestinationVoteRequestedSignature, AMADestinationVoteActor*, AMAPlayerCharacter*);

UCLASS()
class P_MA_API AMADestinationVoteActor : public AActor
{
	GENERATED_BODY()

public:
	AMADestinationVoteActor();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool RequestVote(AMAPlayerCharacter* Interactor);
	bool ApplyVote(APlayerState* PlayerState);
	bool RemoveVote(APlayerState* PlayerState);
	bool ClearVotes();

	FMADestinationVoteRequestedSignature OnVoteRequested;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAHighlightComponent> HighlightComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UWidgetComponent> VoteStatusWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UWidgetComponent> DestinationInfoWidgetComponent;

	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_OptionData, Category="Destination")
	FMADestinationOptionData OptionData;

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void NotifyVoteChanged();
	void RefreshDestinationInfoWidget();
	UTexture2D* ResolveDestinationIcon() const;

	UFUNCTION()
	void OnRep_Voters();

	UFUNCTION()
	void OnRep_OptionData();

	UPROPERTY(Transient, ReplicatedUsing=OnRep_Voters)
	TArray<TObjectPtr<APlayerState>> Voters;
};
