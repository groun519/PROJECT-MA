#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Level/Sector/Spline/SplineSector.h"
#include "BattleSector.generated.h"

UCLASS()
class P_MA_API ABattleSector : public ASplineSector
{
	GENERATED_BODY()
	
public:
	ABattleSector();
};
