#include "Convenience/MAHighlightComponent.h"

#include "Components/PrimitiveComponent.h"

void UMAHighlightComponent::AddTarget(UPrimitiveComponent* Target)
{
	if (!ensureMsgf(Target, TEXT("Highlight target is null."))) return;

	HighlightTargets.AddUnique(Target);
	ApplyHighlight();
}

void UMAHighlightComponent::SetHighlight(
	UObject& Requester,
	bool bEnabled,
	const FLinearColor& Color,
	int32 Priority)
{
	const int32 RemovedCount = HighlightRequests.RemoveAll(
		[&Requester](const FRequest& Request)
		{
			return Request.Requester.Get() == &Requester;
		});
	if (bEnabled)
	{
		HighlightRequests.Add({&Requester, ConvertColorHueToStencilValue(Color), Priority});
	}

	if (bHighlightEnabled && (bEnabled || RemovedCount > 0)) ApplyHighlight();
}

void UMAHighlightComponent::SetHighlightEnabled(const bool bEnabled)
{
	if (bHighlightEnabled == bEnabled) return;

	bHighlightEnabled = bEnabled;
	ApplyHighlight();
}

int32 UMAHighlightComponent::ConvertColorHueToStencilValue(const FLinearColor& Color)
{
	constexpr int32 HueStepCount = 36;
	constexpr float HueStep = 360.f / HueStepCount;
	constexpr int32 WhiteStencil = 37;

	const FLinearColor HSV = Color.LinearRGBToHSV();
	if (HSV.G <= KINDA_SMALL_NUMBER) return WhiteStencil;

	return FMath::RoundToInt(HSV.R / HueStep) % HueStepCount + 1;
}

void UMAHighlightComponent::ApplyHighlight()
{
	HighlightTargets.RemoveAll(
		[](const TWeakObjectPtr<UPrimitiveComponent>& Target)
		{
			return !Target.IsValid();
		});
	HighlightRequests.RemoveAll(
		[](const FRequest& Request)
		{
			return !Request.Requester.IsValid();
		});

	const FRequest* ActiveRequest = nullptr;
	if (bHighlightEnabled)
	{
		for (const FRequest& Request : HighlightRequests)
		{
			if (!ActiveRequest || Request.Priority >= ActiveRequest->Priority)
			{
				ActiveRequest = &Request;
			}
		}
	}

	for (const TWeakObjectPtr<UPrimitiveComponent>& Target : HighlightTargets)
	{
		if (UPrimitiveComponent* Primitive = Target.Get())
		{
			Primitive->SetRenderCustomDepth(ActiveRequest != nullptr);
			if (ActiveRequest) Primitive->SetCustomDepthStencilValue(ActiveRequest->StencilValue);
		}
	}
}
