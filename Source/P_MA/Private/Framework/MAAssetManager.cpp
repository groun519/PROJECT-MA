// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAAssetManager.h"

UMAAssetManager& UMAAssetManager::Get()
{
	UMAAssetManager* Singleton = Cast<UMAAssetManager>(GEngine->AssetManager.Get());
	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogLoad, Fatal, TEXT("Asset Manager Needs to be of the type UMAAssetManager"));
	return (*NewObject<UMAAssetManager>()); // 널 포인터 방지용 더미 반환
}

void UMAAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// 현재는 로드할 것이 없음 (DataTable은 필요할 때 로드하거나 참조됨)
	// UE_LOG(LogTemp, Warning, TEXT("MAAssetManager Initialized"));
}