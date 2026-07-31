#pragma once

#include "CoreMinimal.h"

class UTexture2D;

/** Resolved icon values shared by UI presentation. */
struct FMAIconData
{
	UTexture2D* Icon = nullptr;
	UTexture2D* SubIcon = nullptr;
	FLinearColor IconColor = FLinearColor::White;
	FLinearColor InnerColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.f);
	FLinearColor FrameColor = FLinearColor::White;
};

/** Resolved content identity shared by tooltips, slots, shops, and other UI. */
struct FMADisplayData
{
	FText DisplayName;
	FText Description;
	FMAIconData IconData;
};
