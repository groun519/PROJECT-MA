#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyHubGameMode.generated.h"

class ALobbyHubArrivalVolume;

UCLASS()
class P_MA_API ALobbyHubGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyHubGameMode();

	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	ALobbyHubArrivalVolume* FindArrivalVolume() const;
};
