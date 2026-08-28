#pragma once

#include "CoreMinimal.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "Player/MAPlayerControllerBase.h"
#include "LobbyHubPlayerController.generated.h"

UCLASS()
class P_MA_API ALobbyHubPlayerController : public AMAPlayerControllerBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void AcknowledgePossession(APawn* NewPawn) override;

	/** Routes the owning client's loadout selection to its authoritative PlayerState. */
	void SetLoadoutSelection(const FLoadoutSelection& Loadout);

private:
	void ApplyLoadoutSelection(const FLoadoutSelection& Loadout);

	UFUNCTION(Server, Reliable)
	void ServerApplyLoadoutSelection(const FLoadoutSelection& Loadout);
};
