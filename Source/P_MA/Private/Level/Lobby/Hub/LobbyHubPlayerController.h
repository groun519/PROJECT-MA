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

private:
	void ApplySavedLoadout(const FLoadoutSelection& Loadout);

	UFUNCTION(Server, Reliable)
	void ServerApplySavedLoadout(const FLoadoutSelection& Loadout);
};
