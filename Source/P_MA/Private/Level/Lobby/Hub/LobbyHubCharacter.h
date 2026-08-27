#pragma once

#include "CoreMinimal.h"
#include "Player/MAPlayerCharacter.h"
#include "LobbyHubCharacter.generated.h"

UCLASS()
class P_MA_API ALobbyHubCharacter : public AMAPlayerCharacter
{
	GENERATED_BODY()

public:
	ALobbyHubCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Initializes only the runtime state used by the Hub. Combat initialization is intentionally excluded. */
	void InitializeHubRuntime();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Hub|Movement", meta = (ClampMin = "0.0"))
	float HubMoveSpeed = 375.f;
};
