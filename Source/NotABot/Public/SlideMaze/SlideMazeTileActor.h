#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlideMaze/SlideMazeTypes.h"
#include "SlideMazeTileActor.generated.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API ASlideMazeTileActor : public AActor
{
	GENERATED_BODY()

public:
	ASlideMazeTileActor();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Tile")
	void SetTileType(ESlideMazeTileType NewType);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Tile")
	void SetTileSymbol(const FString& NewSymbol);

	void SetTileSymbol(TCHAR NewSymbol);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Tile")
	void SetBroken();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Tile")
	void UpdateVisual();

	UFUNCTION(BlueprintPure, Category="SlideMaze|Tile")
	UStaticMesh* GetTileMesh() const;

	UFUNCTION(BlueprintPure, Category="SlideMaze|Tile")
	FVector GetTileMeshRelativeScale() const;

	UFUNCTION(BlueprintPure, Category="SlideMaze|Tile")
	UMaterialInterface* GetMaterialForTileType(ESlideMazeTileType Type) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Tile")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Tile")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Tile")
	ESlideMazeTileType TileType = ESlideMazeTileType::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Tile")
	FString TileSymbolString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Tile")
	bool bHideWhenBroken = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> EmptyMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> WallMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> BreakableMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> StartMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> GoalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> TeleportInMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Materials")
	TObjectPtr<UMaterialInterface> TeleportOutMaterial;

private:
	TCHAR TileSymbol = TEXT('□');
};
