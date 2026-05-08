#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillModuleDragDropOperation.generated.h"

UCLASS()
class P_MA_API UMASkillModuleDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EMAAbilityInputID SourceInputID = EMAAbilityInputID::None;

	UPROPERTY(Transient)
	int32 SourceModuleIndex = INDEX_NONE;
};
