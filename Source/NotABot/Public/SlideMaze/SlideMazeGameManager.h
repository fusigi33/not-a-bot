#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlideMaze/SlideMazeTypes.h"
#include "SlideMazeGameManager.generated.h"

class ASceneCapture2D;
class ASlideMazePlayerActor;
class ASlideMazeTileActor;
class UInstancedStaticMeshComponent;
class USceneComponent;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSlideMazeTurnsChangedSignature, int32, Turns);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlideMazeSimpleSignature);

UCLASS()
class NOTABOT_API ASlideMazeGameManager : public AActor
{
	GENERATED_BODY()

public:
	ASlideMazeGameManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void InitializeMaze();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	bool ParseMap();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void SpawnTiles();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void ClearSpawnedActors();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void ResetMaze();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Input")
	void RequestMove(ESlideMazeDirection Direction);

	UFUNCTION(BlueprintPure, Category="SlideMaze|Input")
	bool CanAcceptInput() const;

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Move")
	FSlideMazeMoveResult ResolveSlide(ESlideMazeDirection Direction) const;

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Move")
	void StartPlayerMoveToTile(FIntPoint TargetTile);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Move")
	void FinishPlayerMove();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Move")
	void HandleTeleport(const FString& TeleportSymbolString);

	void HandleTeleport(TCHAR TeleportSymbol);


	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void BreakObstacleAt(FIntPoint TileCoord);

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	bool CheckGoal();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void ConsumeTurn();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void TriggerSuccess();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void TriggerFail();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Capture")
	void CaptureBoard();

	UFUNCTION(BlueprintPure, Category="SlideMaze|Grid")
	FVector GridToWorld(FIntPoint Grid) const;

	UFUNCTION(BlueprintPure, Category="SlideMaze|Grid")
	FIntPoint WorldToGrid(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category="SlideMaze|Grid")
	bool IsInsideGrid(FIntPoint Grid) const;

	TCHAR GetTileCharAt(FIntPoint Grid) const;
	void SetTileCharAt(FIntPoint Grid, TCHAR NewChar);

	UFUNCTION(BlueprintPure, Category="SlideMaze|Grid")
	ESlideMazeTileType GetTileTypeAt(FIntPoint Grid) const;

	UFUNCTION(BlueprintPure, Category="SlideMaze|Grid")
	FIntPoint DirectionToDelta(ESlideMazeDirection Direction) const;

	bool FindTeleportExit(TCHAR EntranceSymbol, FIntPoint& OutExitGrid) const;

	UPROPERTY(BlueprintAssignable, Category="SlideMaze|Events")
	FSlideMazeTurnsChangedSignature OnTurnsChanged;

	UPROPERTY(BlueprintAssignable, Category="SlideMaze|Events")
	FSlideMazeSimpleSignature OnGameSuccess;

	UPROPERTY(BlueprintAssignable, Category="SlideMaze|Events")
	FSlideMazeSimpleSignature OnGameFail;

	UPROPERTY(BlueprintAssignable, Category="SlideMaze|Events")
	FSlideMazeSimpleSignature OnMazeUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Map")
	TArray<FString> MapRows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Map")
	int32 Width = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Map")
	int32 Height = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Rules")
	int32 MaxTurns = 9;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Rules")
	int32 CurrentTurns = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Board")
	float TileSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Board")
	float TileOverlayZOffset = 8.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Board")
	TObjectPtr<USceneComponent> BoardRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Board|Instances")
	TObjectPtr<UInstancedStaticMeshComponent> EmptyTileInstances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Move")
	float MoveDuration = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Rules")
	bool bConsumeTurnOnBlockedMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Capture")
	bool bCaptureEveryMoveOnly = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|State")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|State")
	bool bGameFinished = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Classes")
	TSubclassOf<ASlideMazeTileActor> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Classes")
	TSubclassOf<ASlideMazePlayerActor> PlayerClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="SlideMaze|Capture")
	TObjectPtr<ASceneCapture2D> SceneCaptureActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Capture")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	FVector MoveStartLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	FVector MoveTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	float MoveElapsed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	FIntPoint PendingTargetGrid = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	bool bPendingBreakable = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	FIntPoint PendingBreakableGrid = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Move")
	bool bPendingGoalCheck = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Map")
	FIntPoint StartGrid = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Map")
	FIntPoint GoalGrid = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|State")
	FIntPoint CurrentPlayerGrid = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Actors")
	TMap<FIntPoint, TObjectPtr<ASlideMazeTileActor>> SpawnedTileMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Actors")
	TObjectPtr<ASlideMazePlayerActor> PlayerActor;

private:
	static TCHAR EmptyChar();
	static bool IsTeleportEntrance(TCHAR Symbol);
	static bool IsTeleportExit(TCHAR Symbol);

	void ConfigureFloorInstanceComponent();
	FTransform MakeFloorInstanceTransform(FIntPoint Grid) const;

	FSlideMazeMoveResult PendingMoveResult;
};
