#pragma once

#include "CoreMinimal.h"

struct FMASkillModuleData;

enum class EMASkillModuleDiagnosticSeverity : uint8
{
	Warning,
	Error
};

struct FMASkillModuleDiagnostic
{
	EMASkillModuleDiagnosticSeverity Severity = EMASkillModuleDiagnosticSeverity::Error;
	FString Path;
	FText Message;

	FText ToText() const;
};

struct FMASkillModuleDataValidator
{
	static bool Validate(
		int32 ModuleId,
		const FMASkillModuleData& ModuleData,
		FMASkillModuleDiagnostic& OutDiagnostic);
};
