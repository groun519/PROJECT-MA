#pragma once

#include "Components/PrimitiveComponent.h"

struct FMARenderStencil
{
	static void SetHighlightValue(UPrimitiveComponent& Primitive, const uint8 HighlightValue)
	{
		const uint8 StencilValue =
			(static_cast<uint8>(Primitive.CustomDepthStencilValue) & TransitionBit) |
			(HighlightValue & HighlightMask);
		Apply(Primitive, StencilValue);
	}

	static void SetTransitionVisible(UPrimitiveComponent& Primitive, const bool bVisible)
	{
		uint8 StencilValue = static_cast<uint8>(Primitive.CustomDepthStencilValue);
		StencilValue = bVisible
			? StencilValue | TransitionBit
			: StencilValue & HighlightMask;
		Apply(Primitive, StencilValue);
	}

private:
	static void Apply(UPrimitiveComponent& Primitive, const uint8 StencilValue)
	{
		Primitive.SetCustomDepthStencilValue(StencilValue);
		Primitive.SetCustomDepthStencilWriteMask(
			(StencilValue & TransitionBit) != 0
				? ERendererStencilMask::ERSM_255
				: ERendererStencilMask::ERSM_Default);
		Primitive.SetRenderCustomDepth(StencilValue != 0);
	}

	static constexpr uint8 HighlightMask = 0x7F; // 0111 1111
	static constexpr uint8 TransitionBit = 0x80; // 1000 0000
};
