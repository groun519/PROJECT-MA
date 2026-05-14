#pragma once

#include "CoreMinimal.h"

class APlayerController;
class UEnhancedInputUserSettings;
class UInputAction;
class UInputMappingContext;
struct FKey;

class P_MA_API FMAInputStatics
{
public:
	static void RegisterInputMappingContextDefaults(const APlayerController* PlayerController, const UInputMappingContext* MappingContext);
	static FText GetInputActionText(const APlayerController* PlayerController, const UInputMappingContext* MappingContext, const UInputAction* InputAction);
	static FText GetKeyDisplayText(const FKey& Key);

private:
	static FName GetInputActionMappingName(const UInputMappingContext* MappingContext, const UInputAction* InputAction);
	static UEnhancedInputUserSettings* GetInputUserSettings(const APlayerController* PlayerController);
};
