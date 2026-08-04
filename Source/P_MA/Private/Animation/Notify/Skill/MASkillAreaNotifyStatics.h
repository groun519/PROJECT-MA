#pragma once

#include "CoreMinimal.h"

class USkeletalMeshComponent;
class UWorld;
struct FMASkillEvent;
struct FMASkillWorldAreaShape;

class P_MA_API MASkillAreaNotifyStatics final
{
public:
	static bool ResolveOriginTransform(USkeletalMeshComponent* MeshComp, FTransform& OutOriginTransform);
	static void AppendTargetData(FMASkillEvent& OutEvent, const FMASkillWorldAreaShape& Area);
	static void DrawEditorPreview(UWorld* World, const FMASkillWorldAreaShape& Area);
	static bool IsEditorPreviewWorldNoPIE(const UWorld* World);

private:
	MASkillAreaNotifyStatics() = delete;
};
