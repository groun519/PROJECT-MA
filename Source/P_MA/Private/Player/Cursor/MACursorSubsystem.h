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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCursorTargetRelationChanged, ECursorTargetRelation, NewRelation);

class UMACursorWidget;
class UPrimitiveComponent;

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

private:
	struct FHighlightedPrimitiveState
	{
		TWeakObjectPtr<UPrimitiveComponent> Component;
		bool bPreviousRenderCustomDepth = false;
		int32 PreviousCustomDepthStencilValue = 0;
	};

	struct FHoveredCursorTarget
	{
		TWeakObjectPtr<AActor> Actor;
	};

	void RestartCursorTimer();
	void StopCursorTimer();

	void RefreshCursorTargetRelation();
	FHoveredCursorTarget ResolveHoveredTarget() const;
	ECursorTargetRelation ResolveCursorTargetRelation(AActor* HitActor) const;
	void UpdateHoveredActorHighlight(const FHoveredCursorTarget& HoveredTarget, ECursorTargetRelation InRelation);
	void ClearHoveredActorHighlight();

	void InitializeRuntimeCursorWidget();
	TSubclassOf<UMACursorWidget> ResolveCursorWidgetClass();
	void ApplyCursorRelationColor(ECursorTargetRelation InRelation);

	UPROPERTY(EditDefaultsOnly, Category = "Cursor", meta = (ClampMin = "0.01"))
	float CursorRelationCheckInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FLinearColor CursorFriendlyColor = FLinearColor(0.2f, 1.f, 0.2f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FLinearColor CursorHostileColor = FLinearColor(1.f, 0.2f, 0.2f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FLinearColor CursorNeutralColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Cursor|Highlight")
	int32 HostileHighlightStencil = 250;

	UPROPERTY(EditDefaultsOnly, Category = "Cursor|Highlight")
	int32 NeutralHighlightStencil = 251;

	UPROPERTY(EditDefaultsOnly, Category = "Cursor|Highlight")
	int32 FriendlyHighlightStencil = 252;

	ECursorTargetRelation CursorTargetRelation = ECursorTargetRelation::None;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<UMACursorWidget> CursorWidgetInstance;

	TWeakObjectPtr<AActor> HighlightedActor;
	ECursorTargetRelation HighlightedActorRelation = ECursorTargetRelation::None;
	TArray<FHighlightedPrimitiveState> HighlightedPrimitiveStates;

	FTimerHandle CursorRelationTimerHandle;
};
