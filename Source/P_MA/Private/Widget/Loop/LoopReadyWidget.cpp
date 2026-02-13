// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Loop/LoopReadyWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Framework/MAGameState.h"
#include "Player/MAPlayerState.h"
#include "Player/MAPlayerController.h"

void ULoopReadyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &ULoopReadyWidget::HandleReadyClicked);
	}

	if (AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		GS->OnLoopReadyEntriesChanged.AddUObject(this, &ULoopReadyWidget::HandleLoopReadyEntriesChanged);
	}

	RefreshFromGameState();
}

void ULoopReadyWidget::NativeDestruct()
{
	if (AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		GS->OnLoopReadyEntriesChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}
void ULoopReadyWidget::UpdatePlayerStatuses(const TArray<FLoopReadyPlayerStatus>& Statuses)
{
	EnsureEntryWidgets(Statuses.Num());

	for (int32 Index = 0; Index < Statuses.Num(); ++Index)
	{
		if (StatusWidgets.IsValidIndex(Index) && StatusWidgets[Index])
		{
			StatusWidgets[Index]->SetStatus(Statuses[Index]);
		}
	}
}

void ULoopReadyWidget::HandleReadyClicked()
{
	AMAPlayerController* PC = GetOwningPlayer<AMAPlayerController>();
	if (!PC)
	{
		return;
	}

	AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr;
	APlayerState* PS = GetOwningPlayerState();
	const bool bCurrentReady = (GS && PS) ? GS->GetLoopReadyForPlayer(PS) : false;
	PC->ServerSetLoopReady(!bCurrentReady);
}

void ULoopReadyWidget::HandleLoopReadyEntriesChanged()
{
	RefreshFromGameState();
}

void ULoopReadyWidget::RefreshFromGameState()
{
	TArray<FLoopReadyPlayerStatus> Statuses;
	if (const UWorld* World = GetWorld())
	{
		if (AMAGameState* GS = World->GetGameState<AMAGameState>())
		{
			const TArray<FLoopReadyEntry>& Entries = GS->GetLoopReadyEntries();
			for (const FLoopReadyEntry& Entry : Entries)
			{
				FLoopReadyPlayerStatus Status;
				Status.bReady = Entry.bReady;
				if (const AMAPlayerState* MPS = Cast<AMAPlayerState>(Entry.PlayerState))
				{
					const FMaterialParamDataPair Colors = MPS->GetLoadoutColor();
					Status.BodyColor = Colors.BodyData.Color;
					Status.EyeColor = Colors.EyeData.Color;
				}
				Statuses.Add(Status);
			}
		}
	}

	UpdatePlayerStatuses(Statuses);
}

void ULoopReadyWidget::EnsureEntryWidgets(int32 Count)
{
	if (!PlayerStatusBox || !PlayerStatusWidgetClass)
	{
		return;
	}

	while (StatusWidgets.Num() < Count)
	{
		ULoopPlayerStatusWidget* Entry =
			CreateWidget<ULoopPlayerStatusWidget>(GetWorld(), PlayerStatusWidgetClass);
		if (!Entry)
		{
			break;
		}

		PlayerStatusBox->AddChild(Entry);
		StatusWidgets.Add(Entry);
	}

	for (int32 Index = StatusWidgets.Num() - 1; Index >= Count; --Index)
	{
		if (StatusWidgets[Index])
		{
			PlayerStatusBox->RemoveChild(StatusWidgets[Index]);
		}
		StatusWidgets.RemoveAt(Index);
	}
}
