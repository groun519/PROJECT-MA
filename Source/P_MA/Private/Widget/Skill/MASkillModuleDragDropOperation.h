#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "MASkillModuleDragDropOperation.generated.h"

class UActorComponent;
class UMASkillModuleInstance;

UCLASS()
class P_MA_API UMASkillModuleDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<UActorComponent> SourceOwner;
	const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots = nullptr;
	int32 SourceIndex = INDEX_NONE;
};
