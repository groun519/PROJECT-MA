#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyHubMagicCircle.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class APlayerState;

/** Owns the Hub Ready area and its authoritative player occupancy. */
UCLASS()
class P_MA_API ALobbyHubMagicCircle : public AActor
{
	GENERATED_BODY()

public:
	ALobbyHubMagicCircle();

	bool IsPlayerReady(const APlayerState* PlayerState) const;
	int32 GetReadyCount() const { return ReadyPlayers.Num(); }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> CircleMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USphereComponent> ReadyAreaComponent;

private:
	UFUNCTION()
	void HandleReadyAreaBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleReadyAreaEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	UFUNCTION()
	void HandleReadyPlayerDestroyed(AActor* DestroyedActor);

	void AddReadyPlayer(APlayerState& PlayerState);
	void RemoveReadyPlayer(APlayerState& PlayerState);

	UPROPERTY(Replicated)
	TArray<TObjectPtr<APlayerState>> ReadyPlayers;
};
