#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyHubMagicCircle.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/** Hub anchor for the upcoming Ready and seamless transition features. */
UCLASS()
class P_MA_API ALobbyHubMagicCircle : public AActor
{
	GENERATED_BODY()

public:
	ALobbyHubMagicCircle();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> CircleMeshComponent;
};
