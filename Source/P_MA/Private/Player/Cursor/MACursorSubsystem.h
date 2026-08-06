#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MACursorSubsystem.generated.h"

UENUM(BlueprintType)
enum class ECursorTargetRelation : uint8
{
	None		UMETA(DisplayName = "None"),
	Friendly	UMETA(DisplayName = "Friendly"),
	Hostile		UMETA(DisplayName = "Hostile"),
	Neutral		UMETA(DisplayName = "Neutral")
};

class AMACharacter;
class UMACursorWidget;
class UMAHighlightComponent;

UCLASS()
class P_MA_API UMACursorSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	UFUNCTION(BlueprintPure, Category = "Cursor")
	ECursorTargetRelation GetCursorTargetRelation() const { return CursorTargetRelation; }

	bool GetAimDirection(FVector& OutDirection) const;

private:
	void RestartCursorTimer();
	void StopCursorTimer();

	void RefreshCursorTargetRelation();
	static AMACharacter* ResolveCharacterFromHit(const FHitResult& HitResult);
	bool TraceVisibilityUnderCursor(FHitResult& OutHit) const;
	AMACharacter* ResolveHoveredCharacter() const;
	ECursorTargetRelation ResolveCursorTargetRelation(AActor* HitActor) const;
	void UpdateHoveredActorHighlight(AMACharacter* HoveredCharacter, ECursorTargetRelation InRelation);
	void ClearHoveredActorHighlight();

	void InitializeRuntimeCursorWidget();
	TSubclassOf<UMACursorWidget> ResolveCursorWidgetClass();
	FLinearColor ResolveCursorRelationColor(ECursorTargetRelation InRelation) const;
	void ApplyCursorRelationColor(ECursorTargetRelation InRelation);

	UPROPERTY(EditDefaultsOnly, Category = "Cursor", meta = (ClampMin = "0.01"))
	float CursorRelationCheckInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FLinearColor CursorFriendlyColor = FLinearColor(0.2f, 1.f, 0.2f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FLinearColor CursorHostileColor = FLinearColor(1.f, 0.2f, 0.2f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FLinearColor CursorNeutralColor = FLinearColor::White;

	ECursorTargetRelation CursorTargetRelation = ECursorTargetRelation::None;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<UMACursorWidget> CursorWidgetInstance;

	TWeakObjectPtr<AMACharacter> HighlightedActor;
	TWeakObjectPtr<UMAHighlightComponent> HighlightedComponent;
	ECursorTargetRelation HighlightedActorRelation = ECursorTargetRelation::None;

	FTimerHandle CursorRelationTimerHandle;
};
