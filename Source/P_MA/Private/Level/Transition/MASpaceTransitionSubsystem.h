#pragma once

#include "CoreMinimal.h"
#include "Level/Transition/MASpaceTransitionTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MASpaceTransitionSubsystem.generated.h"

class AMALevelRoot;
class AMAMagicCircle;
class AMAPlayerCharacter;
class AMAPlayerControllerBase;
class UMASpaceTransitionMask;
class UMASpaceLightCollector;
class UMAStreamingLevelLoader;
struct FGameplayTag;

/** Owns the sequence of one same-World transition between two Spaces. */
UCLASS()
class P_MA_API UMASpaceTransitionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Space Transition")
	bool RequestTransition(
		TSoftObjectPtr<UWorld> DestinationMap,
		int32 GenerationSeed = 0);

	void BeginClientPrepare(const FMASpaceTransitionRequest& Request);
	void BeginLocalClose(AMAPlayerCharacter* PlayerCharacter);
	void BeginLocalOpen();
	void AbortLocalTransition();

	void HandleClientProgress(
		AMAPlayerControllerBase& PlayerController,
		const FString& DestinationInstanceIdentity,
		bool bSucceeded);

private:
	enum class EPhase : uint8
	{
		Idle,
		Loading,
		Closing,
		Opening
	};

	bool LoadDestination(const FMASpaceTransitionRequest& Request);
	void HandleDestinationLoaded(AMALevelRoot* LoadedLevel);
	void TryBeginClose();
	void BeginOpen();
	void FinishTransition();
	void AbortTransition();
	void DiscardDestination();
	void ResetTransitionState();

	void CompleteLocalOpen(bool bSucceeded);
	void NotifyServer(bool bSucceeded);
	void PlayTransitionSound(const FGameplayTag& SoundTag, const AMAMagicCircle& Circle) const;

	void MovePlayersToDestination(AMALevelRoot& Source, AMALevelRoot& Destination) const;
	void PromoteDestination();

	UPROPERTY(Transient)
	TObjectPtr<UMAStreamingLevelLoader> LevelLoader;

	UPROPERTY(Transient)
	TObjectPtr<UMASpaceTransitionMask> TransitionMask;

	UPROPERTY(Transient)
	TObjectPtr<UMASpaceLightCollector> SourceLightCollector;

	UPROPERTY(Transient)
	TObjectPtr<UMASpaceLightCollector> DestinationLightCollector;

	EPhase Phase = EPhase::Idle;
	float TransitionAlpha = 1.f;
	FMASpaceTransitionRequest ActiveRequest;

	TWeakObjectPtr<AMALevelRoot> CurrentLevel;
	TWeakObjectPtr<AMALevelRoot> DestinationLevel;
	TWeakObjectPtr<AMAPlayerCharacter> LocalPlayerCharacter;

	TSet<TWeakObjectPtr<AMAPlayerControllerBase>> PendingPlayers;

	static constexpr float TransitionDuration = 2.25f;
};
