#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "MAGameplaySoundSubsystem.generated.h"

class AActor;
class UMAGameplaySoundLibrary;

UCLASS()
class P_MA_API UMAGameplaySoundSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void PlayAtLocation(
		FGameplayTag SoundTag,
		const FVector& Location,
		const AActor* OwningActor);

private:
	void ReportInvalidMappingOnce(FGameplayTag SoundTag, const TCHAR* Reason);

	UPROPERTY(Transient)
	TObjectPtr<UMAGameplaySoundLibrary> GameplaySoundLibrary;

	TSet<FGameplayTag> ReportedInvalidMappings;
};
