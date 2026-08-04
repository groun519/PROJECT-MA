#include "Input/MAInputStatics.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "UserSettings/EnhancedInputUserSettings.h"

void FMAInputStatics::RegisterInputMappingContextDefaults(const APlayerController* PlayerController, const UInputMappingContext* MappingContext)
{
	if (!MappingContext) return;

	if (UEnhancedInputUserSettings* UserSettings = GetInputUserSettings(PlayerController))
	{
		UserSettings->RegisterInputMappingContext(MappingContext);
	}
}

FText FMAInputStatics::GetInputActionText(
	const APlayerController* PlayerController,
	const UInputMappingContext* MappingContext,
	const UInputAction* InputAction)
{
	if (!ensureMsgf(PlayerController && MappingContext && InputAction,
		TEXT("GetInputActionText requires PlayerController, MappingContext, and InputAction.")))
	{
		return FText::GetEmpty();
	}

	UEnhancedInputUserSettings* UserSettings = GetInputUserSettings(PlayerController);
	if (!ensureMsgf(UserSettings, TEXT("Input user settings are not available for '%s'."), *GetNameSafe(PlayerController)))
	{
		return FText::GetEmpty();
	}

	const FName MappingName = GetInputActionMappingName(MappingContext, InputAction);
	if (!ensureMsgf(!MappingName.IsNone(), TEXT("InputAction '%s' is not player-mappable in MappingContext '%s'."),
		*GetNameSafe(InputAction), *GetNameSafe(MappingContext)))
	{
		return FText::GetEmpty();
	}

	const FPlayerKeyMapping* CurrentMapping = UserSettings->FindCurrentMappingForSlot(MappingName, EPlayerMappableKeySlot::First);
	if (!ensureMsgf(CurrentMapping, TEXT("No current key mapping found for '%s'."), *MappingName.ToString()))
	{
		return FText::GetEmpty();
	}

	const FKey Key = CurrentMapping->GetCurrentKey();
	return Key.IsValid() ? GetKeyDisplayText(Key) : FText::GetEmpty();
}

FText FMAInputStatics::GetKeyDisplayText(const FKey& Key)
{
	if (Key == EKeys::RightMouseButton) return FText::FromString(TEXT("RMB"));
	if (Key == EKeys::LeftMouseButton) return FText::FromString(TEXT("LMB"));
	if (Key == EKeys::MiddleMouseButton) return FText::FromString(TEXT("MMB"));
	if (Key == EKeys::ThumbMouseButton) return FText::FromString(TEXT("Mouse4"));
	if (Key == EKeys::ThumbMouseButton2) return FText::FromString(TEXT("Mouse5"));
	if (Key == EKeys::SpaceBar) return FText::FromString(TEXT("Space"));

	return Key.GetDisplayName(false);
}

FName FMAInputStatics::GetInputActionMappingName(const UInputMappingContext* MappingContext, const UInputAction* InputAction)
{
	if (!MappingContext || !InputAction) return NAME_None;

	for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
	{
		if (Mapping.Action != InputAction) continue;
		if (!Mapping.IsPlayerMappable()) continue;

		const FName MappingName = Mapping.GetMappingName();
		if (!MappingName.IsNone()) return MappingName;
	}

	return NAME_None;
}

UEnhancedInputUserSettings* FMAInputStatics::GetInputUserSettings(const APlayerController* PlayerController)
{
	if (!PlayerController) return nullptr;

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return nullptr;

	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	return InputSubsystem ? InputSubsystem->GetUserSettings() : nullptr;
}
