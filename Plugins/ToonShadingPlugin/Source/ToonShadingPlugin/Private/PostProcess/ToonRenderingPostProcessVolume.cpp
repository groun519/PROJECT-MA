// Fill out your copyright notice in the Description page of Project Settings.


#include "PostProcess/ToonRenderingPostProcessVolume.h"

AToonRenderingPostProcessVolume::AToonRenderingPostProcessVolume()
{
	bUnbound = true;
	
	Settings.bOverride_BloomIntensity = true;
	Settings.BloomIntensity = 0.0f;
	
	Settings.bOverride_AutoExposureMinBrightness = true;
	Settings.bOverride_AutoExposureMaxBrightness = true;
	Settings.AutoExposureMinBrightness = 1.0f;
	Settings.AutoExposureMaxBrightness = 1.0f;
	
	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = 0.0f;
	
	Settings.bOverride_SceneFringeIntensity = true;
	Settings.SceneFringeIntensity = 0.0f;
}
