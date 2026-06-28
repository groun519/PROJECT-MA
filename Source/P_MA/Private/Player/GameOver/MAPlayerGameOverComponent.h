#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MAPlayerGameOverComponent.generated.h"

class UMAGameOverWidget;
enum class EMAGameOverAction : uint8;

UCLASS(ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAPlayerGameOverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAPlayerGameOverComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleGameOverChanged(bool bGameOver);
	void ShowGameOverWidget();
	void RemoveGameOverWidget();
	void HandleGameOverActionRequested(EMAGameOverAction Action);

	UFUNCTION(Server, Reliable)
	void ServerRequestReturnToLobby();

	UPROPERTY(EditDefaultsOnly, Category = "Game Over")
	TSubclassOf<UMAGameOverWidget> GameOverWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMAGameOverWidget> GameOverWidget;
};
