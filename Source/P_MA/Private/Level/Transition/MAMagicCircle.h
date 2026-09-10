#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MAMagicCircle.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class APlayerState;

/** Detects players inside the Magic Circle and provides its transform anchor. */
UCLASS()
class P_MA_API AMAMagicCircle : public AActor
{
	GENERATED_BODY()

public:
	AMAMagicCircle();

	/** Player Detection **/
	bool IsPlayerInCircle(const APlayerState* PlayerState) const;
	int32 GetPlayerCount() const { return PlayersInCircle.Num(); }

	/** Transform **/
	FTransform WorldToCircleTransform(const FTransform& WorldTransform) const;
	FTransform CircleToWorldTransform(const FTransform& CircleTransform) const;

	/** Replication **/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Player Detection **/
	UFUNCTION()
	void HandlePlayerAreaBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandlePlayerAreaEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	UFUNCTION()
	void HandlePlayerInCircleDestroyed(AActor* DestroyedActor);

	void AddPlayerInCircle(APlayerState& PlayerState);
	void RemovePlayerInCircle(APlayerState& PlayerState);

	/** Components **/
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> CircleMeshComponent;

	/** Player Detection **/
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USphereComponent> PlayerAreaComponent;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<APlayerState>> PlayersInCircle;
};
