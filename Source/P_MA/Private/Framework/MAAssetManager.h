// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "MAAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class UMAAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UMAAssetManager& Get();

	// 나중에 데이터 테이블 비동기 로드 등이 필요하면 여기에 추가
	virtual void StartInitialLoading() override;
};