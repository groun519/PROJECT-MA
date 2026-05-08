#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"

class APlayerController;
class AMAPlayerCharacter;
class UEnhancedInputUserSettings;
class UInputMappingContext;
struct FKey;

class P_MA_API FMAInputStatics
{
public:
	static void RegisterInputMappingContextDefaults(const APlayerController* PlayerController, const UInputMappingContext* MappingContext);
	static FText GetGameplayAbilityInputText(const APlayerController* PlayerController, const AMAPlayerCharacter* PlayerCharacter, EMAAbilityInputID InputID);
	static FText GetKeyDisplayText(const FKey& Key);

private:
	static FName GetGameplayAbilityMappingName(const AMAPlayerCharacter* PlayerCharacter, EMAAbilityInputID InputID);
	static UEnhancedInputUserSettings* GetInputUserSettings(const APlayerController* PlayerController);
};
