#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MAStreamingLevelLoader.generated.h"

class AMALevelRoot;
class ULevel;
class ULevelStreaming;
class UWorld;

DECLARE_DELEGATE_OneParam(FOnMALevelLoaded, AMALevelRoot*);
DECLARE_DELEGATE_OneParam(FOnMALevelPreparing, ULevel&);

/** Owns the streaming implementation that turns a Level map into an AMALevelRoot. */
UCLASS()
class P_MA_API UMAStreamingLevelLoader : public UObject
{
	GENERATED_BODY()

public:
	// Registers the initial streaming level that was loaded outside this loader.
	AMALevelRoot* RegisterInitialLevel();
	// Chooses the opposite runtime placement from this loaded Level's placement.
	FTransform GetSwapTransform(AMALevelRoot& CurrentLevel) const;
	bool LoadLevel(
		TSoftObjectPtr<UWorld> LevelMap,
		const FTransform& InstanceTransform,
		const FString& InstanceIdentity,
		FOnMALevelLoaded OnLoaded,
		FOnMALevelPreparing OnPreparing = FOnMALevelPreparing());
	void UnloadLevel(AMALevelRoot& LevelRoot);
	void CancelPendingLoad();

private:
	// Runs before the loaded Level starts registering its components for display.
	UFUNCTION()
	void HandleLevelLoaded();

	UFUNCTION()
	void HandleLevelShown();

	AMALevelRoot* FindLevelRoot(ULevelStreaming& StreamingLevel) const;
	void ReleaseStreamingLevel(ULevelStreaming& StreamingLevel) const;

	TMap<TWeakObjectPtr<AMALevelRoot>, TWeakObjectPtr<ULevelStreaming>> LoadedLevels;
	TWeakObjectPtr<ULevelStreaming> PendingStreamingLevel;
	FOnMALevelLoaded PendingLoadedDelegate;
	FOnMALevelPreparing PendingPreparingDelegate;
};
