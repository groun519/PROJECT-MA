#include "Level/Streaming/MAStreamingLevelLoader.h"

#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "Level/Streaming/MALevelRoot.h"

DEFINE_LOG_CATEGORY_STATIC(LogMAStreamingLevelLoader, Log, All);

AMALevelRoot* UMAStreamingLevelLoader::RegisterInitialLevel()
{
	UWorld* World = GetTypedOuter<UWorld>();
	if (!World) return nullptr;

	AMALevelRoot* FoundLevelRoot = nullptr;
	ULevelStreaming* FoundStreamingLevel = nullptr;
	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		if (!StreamingLevel || !StreamingLevel->IsLevelVisible()) continue;
		ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
		if (!LoadedLevel) continue;

		for (AActor* Actor : LoadedLevel->Actors)
		{
			AMALevelRoot* LevelRoot = Cast<AMALevelRoot>(Actor);
			if (!LevelRoot) continue;

			if (!ensureMsgf(
				!FoundLevelRoot,
				TEXT("The initial World must contain exactly one visible LevelRoot.")))
			{
				return nullptr;
			}

			FoundLevelRoot = LevelRoot;
			FoundStreamingLevel = StreamingLevel;
		}
	}

	if (!ensureMsgf(FoundLevelRoot, TEXT("The initial World requires one visible LevelRoot."))) return nullptr;

	LoadedLevels.Add(FoundLevelRoot, FoundStreamingLevel);
	return FoundLevelRoot;
}

FTransform UMAStreamingLevelLoader::GetSwapTransform(AMALevelRoot& CurrentLevel) const
{
	const ULevelStreaming* StreamingLevel = LoadedLevels.FindChecked(&CurrentLevel).Get();
	check(StreamingLevel);
	const double CurrentX = StreamingLevel->LevelTransform.GetLocation().X;
	return FTransform(FVector(FMath::IsNearlyZero(CurrentX) ? 100000.0 : 0.0, 0.0, 0.0));
}

bool UMAStreamingLevelLoader::LoadLevel(
	TSoftObjectPtr<UWorld> LevelMap,
	const FTransform& InstanceTransform,
	const FString& InstanceIdentity,
	FOnMALevelLoaded OnLoaded,
	FOnMALevelPreparing OnPreparing)
{
	if (LevelMap.IsNull()) return false;
	if (!ensureMsgf(!PendingStreamingLevel.IsValid(), TEXT("Only one Level can load at a time."))) return false;

	bool bLoadStarted = false;
	ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this,
		LevelMap,
		InstanceTransform,
		bLoadStarted,
		InstanceIdentity);
	if (!bLoadStarted || !StreamingLevel)
	{
		UE_LOG(LogMAStreamingLevelLoader, Error, TEXT("Failed to stream Level '%s'."), *LevelMap.ToString());
		return false;
	}

	PendingStreamingLevel = StreamingLevel;
	PendingLoadedDelegate = MoveTemp(OnLoaded);
	PendingPreparingDelegate = MoveTemp(OnPreparing);
	StreamingLevel->OnLevelLoaded.AddUniqueDynamic(this, &UMAStreamingLevelLoader::HandleLevelLoaded);
	StreamingLevel->OnLevelShown.AddUniqueDynamic(this, &UMAStreamingLevelLoader::HandleLevelShown);
	if (StreamingLevel->IsLevelLoaded()) HandleLevelLoaded();
	if (StreamingLevel->IsLevelVisible()) HandleLevelShown();
	return true;
}

void UMAStreamingLevelLoader::UnloadLevel(AMALevelRoot& LevelRoot)
{
	TWeakObjectPtr<ULevelStreaming>* StreamingLevel = LoadedLevels.Find(&LevelRoot);
	if (!StreamingLevel) return;

	if (ULevelStreaming* Level = StreamingLevel->Get()) ReleaseStreamingLevel(*Level);
	LoadedLevels.Remove(&LevelRoot);
}

void UMAStreamingLevelLoader::CancelPendingLoad()
{
	if (ULevelStreaming* StreamingLevel = PendingStreamingLevel.Get())
	{
		StreamingLevel->OnLevelLoaded.RemoveDynamic(this, &UMAStreamingLevelLoader::HandleLevelLoaded);
		StreamingLevel->OnLevelShown.RemoveDynamic(this, &UMAStreamingLevelLoader::HandleLevelShown);
		ReleaseStreamingLevel(*StreamingLevel);
	}

	PendingStreamingLevel.Reset();
	PendingLoadedDelegate.Unbind();
	PendingPreparingDelegate.Unbind();
}

void UMAStreamingLevelLoader::HandleLevelLoaded()
{
	ULevelStreaming* StreamingLevel = PendingStreamingLevel.Get();
	if (!StreamingLevel) return;

	StreamingLevel->OnLevelLoaded.RemoveDynamic(this, &UMAStreamingLevelLoader::HandleLevelLoaded);
	FOnMALevelPreparing PreparingDelegate = MoveTemp(PendingPreparingDelegate);
	PendingPreparingDelegate.Unbind();
	PreparingDelegate.ExecuteIfBound(*StreamingLevel->GetLoadedLevel());
}

void UMAStreamingLevelLoader::HandleLevelShown()
{
	ULevelStreaming* StreamingLevel = PendingStreamingLevel.Get();
	if (!StreamingLevel) return;

	StreamingLevel->OnLevelShown.RemoveDynamic(this, &UMAStreamingLevelLoader::HandleLevelShown);
	AMALevelRoot* LevelRoot = FindLevelRoot(*StreamingLevel);
	FOnMALevelLoaded LoadedDelegate = MoveTemp(PendingLoadedDelegate);
	PendingStreamingLevel.Reset();
	PendingLoadedDelegate.Unbind();

	if (!LevelRoot)
	{
		ReleaseStreamingLevel(*StreamingLevel);
		LoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	LoadedLevels.Add(LevelRoot, StreamingLevel);
	LoadedDelegate.ExecuteIfBound(LevelRoot);
}

AMALevelRoot* UMAStreamingLevelLoader::FindLevelRoot(ULevelStreaming& StreamingLevel) const
{
	ULevel* LoadedLevel = StreamingLevel.GetLoadedLevel();
	if (!LoadedLevel) return nullptr;

	AMALevelRoot* FoundLevelRoot = nullptr;
	for (AActor* Actor : LoadedLevel->Actors)
	{
		AMALevelRoot* LevelRoot = Cast<AMALevelRoot>(Actor);
		if (!LevelRoot) continue;

		if (!ensureMsgf(
			!FoundLevelRoot,
			TEXT("Level '%s' must contain exactly one AMALevelRoot."),
			*LoadedLevel->GetName()))
		{
			return nullptr;
		}
		FoundLevelRoot = LevelRoot;
	}

	ensureMsgf(FoundLevelRoot, TEXT("Level '%s' requires one AMALevelRoot."), *LoadedLevel->GetName());
	return FoundLevelRoot;
}

void UMAStreamingLevelLoader::ReleaseStreamingLevel(ULevelStreaming& StreamingLevel) const
{
	StreamingLevel.SetShouldBeVisible(false);
	StreamingLevel.SetShouldBeLoaded(false);
	StreamingLevel.SetIsRequestingUnloadAndRemoval(true);
}
