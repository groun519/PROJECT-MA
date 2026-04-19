#pragma once

#include "CoreMinimal.h"
#include "Level/Sector/Spline/SplineSector.h"
#include "MirrorSplineSector.generated.h"

class UInstancedStaticMeshComponent;
class UPCGComponent;

UCLASS()
class P_MA_API AMirrorSplineSector : public ASplineSector
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetRandomSeed(int32 MaxValue = INT32_MAX) override;

	UFUNCTION(BlueprintCallable, Category = "Mirror")
	void SetSourceSector(ASplineSector* InSourceSector);

	UFUNCTION(BlueprintCallable, Category = "Mirror")
	void AddSourceSector(ASplineSector* InSourceSector);

	UFUNCTION(BlueprintCallable, Category = "Mirror")
	void ClearSourceSectors();

	UFUNCTION(BlueprintCallable, Category = "Mirror")
	void RebuildFromSource();

protected:
	void RebuildFromSourceSector(ASplineSector* InSourceSector);
	void CopySplineFromSource(const ASplineSector* InSourceSector);
	void CopyISMComponentsFromSource(const ASplineSector* InSourceSector);
	void ClearCopiedISMComponents();
	void RebindSourceDelegates();
	void UnbindSourceDelegates();

	void HandleSourceSectorUpdated(ASplineSector* InUpdatedSector);

	UFUNCTION()
	void HandleSourcePCGGenerated(UPCGComponent* InPCGComponent);

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mirror")
	TArray<TObjectPtr<ASplineSector>> SourceSectors;

	UPROPERTY()
	TArray<TWeakObjectPtr<ASplineSector>> BoundSourceSectors;

	UPROPERTY()
	TWeakObjectPtr<ASplineSector> ActiveSourceSector;

	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CopiedISMComponents;
};
