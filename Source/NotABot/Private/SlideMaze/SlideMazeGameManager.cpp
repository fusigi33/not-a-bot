#include "SlideMaze/SlideMazeGameManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SlideMaze/SlideMazePlayerActor.h"
#include "SlideMaze/SlideMazeTileActor.h"
#include "Engine/SceneCapture2D.h"

ASlideMazeGameManager::ASlideMazeGameManager()
{
	PrimaryActorTick.bCanEverTick = true;

	BoardRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BoardRoot"));
	SetRootComponent(BoardRoot);

	EmptyTileInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("EmptyTileInstances"));
	EmptyTileInstances->SetupAttachment(BoardRoot);

	MapRows = {
		TEXT("################"),
		TEXT("#□X#□□□□□A□#□□G#"),
		TEXT("#□□#□□X□#□□X□□□#"),
		TEXT("#□#□X□#□□□□□B□□#"),
		TEXT("#□□#□□□a□□#□□□##"),
		TEXT("#X□□#□□□□□□□□b□#"),
		TEXT("#□□#□□□□□X□#□□X#"),
		TEXT("#S□□□□##□□X□#□□#"),
		TEXT("################")
	};
}

void ASlideMazeGameManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeMaze();
}

void ASlideMazeGameManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsMoving || !PlayerActor)
	{
		return;
	}

	MoveElapsed += DeltaSeconds;
	const float Duration = FMath::Max(MoveDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(MoveElapsed / Duration, 0.0f, 1.0f);
	PlayerActor->SetActorLocation(FMath::Lerp(MoveStartLocation, MoveTargetLocation, Alpha));

	if (Alpha >= 1.0f)
	{
		FinishPlayerMove();
	}
}

void ASlideMazeGameManager::InitializeMaze()
{
	bGameFinished = false;
	bIsMoving = false;
	CurrentTurns = MaxTurns;
	PendingMoveResult = FSlideMazeMoveResult();

	ClearSpawnedActors();

	if (!ParseMap())
	{
		UE_LOG(LogTemp, Error, TEXT("SlideMaze InitializeMaze failed because map parsing failed."));
		return;
	}

	SpawnTiles();

	UClass* SpawnPlayerClass = PlayerClass ? PlayerClass.Get() : ASlideMazePlayerActor::StaticClass();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PlayerActor = GetWorld()->SpawnActor<ASlideMazePlayerActor>(SpawnPlayerClass, GridToWorld(StartGrid), FRotator::ZeroRotator, SpawnParameters);
	if (PlayerActor)
	{
		CurrentPlayerGrid = StartGrid;
		PlayerActor->SetGridPosition(CurrentPlayerGrid, GridToWorld(CurrentPlayerGrid));
	}

	OnTurnsChanged.Broadcast(CurrentTurns);
	OnMazeUpdated.Broadcast();
	CaptureBoard();
}

bool ASlideMazeGameManager::ParseMap()
{
	if (MapRows.Num() != Height)
	{
		UE_LOG(LogTemp, Error, TEXT("SlideMaze map row count must be %d, but is %d."), Height, MapRows.Num());
		return false;
	}

	bool bFoundStart = false;
	bool bFoundGoal = false;
	TSet<TCHAR> Entrances;
	TSet<TCHAR> Exits;

	for (int32 Row = 0; Row < Height; ++Row)
	{
		if (MapRows[Row].Len() != Width)
		{
			UE_LOG(LogTemp, Error, TEXT("SlideMaze row %d length must be %d, but is %d."), Row, Width, MapRows[Row].Len());
			return false;
		}

		for (int32 Col = 0; Col < Width; ++Col)
		{
			const TCHAR Symbol = MapRows[Row][Col];
			const FIntPoint Grid(Col, Row);

			if (Symbol == TEXT('S'))
			{
				StartGrid = Grid;
				bFoundStart = true;
			}
			else if (Symbol == TEXT('G'))
			{
				GoalGrid = Grid;
				bFoundGoal = true;
			}
			else if (IsTeleportEntrance(Symbol))
			{
				Entrances.Add(Symbol);
			}
			else if (IsTeleportExit(Symbol))
			{
				Exits.Add(Symbol);
			}
			else if (Symbol != TEXT('#') && Symbol != TEXT('X') && Symbol != EmptyChar())
			{
				UE_LOG(LogTemp, Error, TEXT("SlideMaze unsupported tile symbol '%c' at row %d col %d."), Symbol, Row, Col);
				return false;
			}
		}
	}

	for (TCHAR Entrance : Entrances)
	{
		const TCHAR Exit = FChar::ToLower(Entrance);
		if (!Exits.Contains(Exit))
		{
			UE_LOG(LogTemp, Error, TEXT("SlideMaze teleport entrance '%c' has no matching exit '%c'."), Entrance, Exit);
			return false;
		}
	}

	if (!bFoundStart || !bFoundGoal)
	{
		UE_LOG(LogTemp, Error, TEXT("SlideMaze map requires one S start and one G goal."));
		return false;
	}

	return true;
}

void ASlideMazeGameManager::SpawnTiles()
{
	if (!GetWorld())
	{
		return;
	}

	ConfigureFloorInstanceComponent();

	UClass* SpawnTileClass = TileClass ? TileClass.Get() : ASlideMazeTileActor::StaticClass();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 Row = 0; Row < Height; ++Row)
	{
		for (int32 Col = 0; Col < Width; ++Col)
		{
			const FIntPoint Grid(Col, Row);
			if (EmptyTileInstances)
			{
				EmptyTileInstances->AddInstance(MakeFloorInstanceTransform(Grid));
			}

			const ESlideMazeTileType TileType = GetTileTypeAt(Grid);
			if (TileType == ESlideMazeTileType::Empty)
			{
				continue;
			}

			ASlideMazeTileActor* Tile = GetWorld()->SpawnActor<ASlideMazeTileActor>(
				SpawnTileClass,
				GridToWorld(Grid) + FVector(0.0f, 0.0f, TileOverlayZOffset),
				FRotator::ZeroRotator,
				SpawnParameters);
			if (!Tile)
			{
				continue;
			}

			const float ScaleXY = TileSize / 100.0f;
			Tile->SetActorScale3D(FVector(ScaleXY, ScaleXY, 1.0f));
			Tile->SetTileSymbol(GetTileCharAt(Grid));
			Tile->SetTileType(TileType);
			SpawnedTileMap.Add(Grid, Tile);
		}
	}
}

void ASlideMazeGameManager::ClearSpawnedActors()
{
	if (EmptyTileInstances)
	{
		EmptyTileInstances->ClearInstances();
	}

	for (TPair<FIntPoint, TObjectPtr<ASlideMazeTileActor>>& Pair : SpawnedTileMap)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}
	SpawnedTileMap.Reset();

	if (IsValid(PlayerActor))
	{
		PlayerActor->Destroy();
	}
	PlayerActor = nullptr;
}

void ASlideMazeGameManager::ResetMaze()
{
	InitializeMaze();
}

void ASlideMazeGameManager::RequestMove(ESlideMazeDirection Direction)
{
	if (!CanAcceptInput())
	{
		return;
	}

	const FSlideMazeMoveResult Result = ResolveSlide(Direction);
	if (!Result.bCanMove)
	{
		if (bConsumeTurnOnBlockedMove)
		{
			ConsumeTurn();
			if (CurrentTurns <= 0 && !CheckGoal())
			{
				TriggerFail();
			}
			CaptureBoard();
		}
		return;
	}

	ConsumeTurn();
	PendingMoveResult = Result;
	bPendingBreakable = Result.bHitBreakable;
	PendingBreakableGrid = Result.BreakableGrid;
	bPendingGoalCheck = true;
	StartPlayerMoveToTile(Result.StopGrid);
}

bool ASlideMazeGameManager::CanAcceptInput() const
{
	return !bIsMoving && !bGameFinished && PlayerActor != nullptr;
}

FSlideMazeMoveResult ASlideMazeGameManager::ResolveSlide(ESlideMazeDirection Direction) const
{
	FSlideMazeMoveResult Result;
	Result.StartGrid = CurrentPlayerGrid;
	Result.StopGrid = CurrentPlayerGrid;

	const FIntPoint Delta = DirectionToDelta(Direction);
	FIntPoint Cursor = CurrentPlayerGrid;

	while (true)
	{
		const FIntPoint Next = Cursor + Delta;
		if (!IsInsideGrid(Next))
		{
			Result.StopGrid = Cursor;
			break;
		}

		const TCHAR Symbol = GetTileCharAt(Next);
		if (Symbol == TEXT('#'))
		{
			Result.StopGrid = Cursor;
			break;
		}

		if (Symbol == TEXT('X'))
		{
			Result.StopGrid = Cursor;
			Result.bHitBreakable = true;
			Result.BreakableGrid = Next;
			break;
		}

		if (IsTeleportEntrance(Symbol))
		{
			FIntPoint ExitGrid;
			if (!FindTeleportExit(Symbol, ExitGrid))
			{
				UE_LOG(LogTemp, Warning, TEXT("SlideMaze teleport '%c' has no exit at runtime."), Symbol);
				Result.StopGrid = Cursor;
				break;
			}

			Result.StopGrid = ExitGrid;
			Result.bHitTeleport = true;
			Result.TeleportEntranceGrid = Next;
			Result.TeleportExitGrid = ExitGrid;
			Result.TeleportSymbol = Symbol;
			Result.TeleportSymbolString = FString::Chr(Symbol);
			break;
		}

		Cursor = Next;
		Result.StopGrid = Cursor;

		if (Symbol == TEXT('G'))
		{
			Result.bReachedGoal = true;
			break;
		}
	}

	Result.bCanMove = Result.StopGrid != Result.StartGrid || Result.bHitBreakable || Result.bHitTeleport || Result.bReachedGoal;
	return Result;
}

void ASlideMazeGameManager::StartPlayerMoveToTile(FIntPoint TargetTile)
{
	if (!PlayerActor)
	{
		return;
	}

	PendingTargetGrid = TargetTile;
	MoveStartLocation = PlayerActor->GetActorLocation();
	MoveTargetLocation = GridToWorld(TargetTile);
	MoveElapsed = 0.0f;
	bIsMoving = true;

	if (MoveDuration <= 0.0f)
	{
		PlayerActor->SetActorLocation(MoveTargetLocation);
		FinishPlayerMove();
	}
}

void ASlideMazeGameManager::FinishPlayerMove()
{
	if (!PlayerActor)
	{
		bIsMoving = false;
		return;
	}

	bIsMoving = false;
	CurrentPlayerGrid = PendingTargetGrid;
	PlayerActor->SetGridPosition(CurrentPlayerGrid, GridToWorld(CurrentPlayerGrid));

	if (bPendingBreakable)
	{
		BreakObstacleAt(PendingBreakableGrid);
	}

	// 현재 구현은 텔레포트 입구에서 출구까지 한 번에 보간한다.
	// 입구 도착 후 순간이동 연출이 필요하면 PendingMoveResult의 Entrance/Exit를 이용해 2단계 이동으로 확장한다.
	if (PendingMoveResult.bHitTeleport)
	{
		HandleTeleport(PendingMoveResult.TeleportSymbol);
	}

	const bool bReachedGoal = bPendingGoalCheck && CheckGoal();
	if (bReachedGoal)
	{
		TriggerSuccess();
	}
	else if (CurrentTurns <= 0)
	{
		TriggerFail();
	}

	bPendingBreakable = false;
	bPendingGoalCheck = false;
	PendingMoveResult = FSlideMazeMoveResult();

	OnMazeUpdated.Broadcast();
	CaptureBoard();
}

void ASlideMazeGameManager::HandleTeleport(const FString& TeleportSymbolString)
{
	if (TeleportSymbolString.Len() > 0)
	{
		HandleTeleport(TeleportSymbolString[0]);
	}
}

void ASlideMazeGameManager::HandleTeleport(TCHAR TeleportSymbol)
{
	FIntPoint ExitGrid;
	if (FindTeleportExit(TeleportSymbol, ExitGrid))
	{
		CurrentPlayerGrid = ExitGrid;
		if (PlayerActor)
		{
			PlayerActor->SetGridPosition(CurrentPlayerGrid, GridToWorld(CurrentPlayerGrid));
		}
	}
}
void ASlideMazeGameManager::BreakObstacleAt(FIntPoint TileCoord)
{
	if (!IsInsideGrid(TileCoord) || GetTileCharAt(TileCoord) != TEXT('X'))
	{
		return;
	}

	SetTileCharAt(TileCoord, EmptyChar());

	if (TObjectPtr<ASlideMazeTileActor>* TilePtr = SpawnedTileMap.Find(TileCoord))
	{
		if (ASlideMazeTileActor* Tile = TilePtr->Get())
		{
			Tile->SetBroken();
		}
	}
}

bool ASlideMazeGameManager::CheckGoal()
{
	if (CurrentPlayerGrid == GoalGrid)
	{
		return true;
	}

	return IsInsideGrid(CurrentPlayerGrid) && GetTileCharAt(CurrentPlayerGrid) == TEXT('G');
}

void ASlideMazeGameManager::ConsumeTurn()
{
	CurrentTurns = FMath::Max(0, CurrentTurns - 1);
	OnTurnsChanged.Broadcast(CurrentTurns);
}

void ASlideMazeGameManager::TriggerSuccess()
{
	if (bGameFinished)
	{
		return;
	}

	bGameFinished = true;
	bIsMoving = false;
	UE_LOG(LogTemp, Log, TEXT("SlideMaze success."));
	OnGameSuccess.Broadcast();
}

void ASlideMazeGameManager::TriggerFail()
{
	if (bGameFinished)
	{
		return;
	}

	bGameFinished = true;
	bIsMoving = false;
	UE_LOG(LogTemp, Log, TEXT("SlideMaze fail."));
	OnGameFail.Broadcast();
}

void ASlideMazeGameManager::CaptureBoard()
{
	if (!SceneCaptureActor)
	{
		return;
	}

	USceneCaptureComponent2D* CaptureComponent = SceneCaptureActor->GetCaptureComponent2D();
	if (!CaptureComponent)
	{
		return;
	}

	if (RenderTarget)
	{
		CaptureComponent->TextureTarget = RenderTarget;
	}

	if (bCaptureEveryMoveOnly || !CaptureComponent->bCaptureEveryFrame)
	{
		CaptureComponent->CaptureScene();
	}
}

FVector ASlideMazeGameManager::GridToWorld(FIntPoint Grid) const
{
	const float WorldX = (static_cast<float>(Grid.X) - static_cast<float>(Width) * 0.5f + 0.5f) * TileSize;
	const float WorldY = (static_cast<float>(Grid.Y) - static_cast<float>(Height) * 0.5f + 0.5f) * TileSize;
	return GetActorLocation() + FVector(WorldX, WorldY, 0.0f);
}

FIntPoint ASlideMazeGameManager::WorldToGrid(FVector WorldLocation) const
{
	const FVector Local = WorldLocation - GetActorLocation();
	const int32 Col = FMath::RoundToInt(Local.X / TileSize + static_cast<float>(Width) * 0.5f - 0.5f);
	const int32 Row = FMath::RoundToInt(Local.Y / TileSize + static_cast<float>(Height) * 0.5f - 0.5f);
	return FIntPoint(Col, Row);
}

bool ASlideMazeGameManager::IsInsideGrid(FIntPoint Grid) const
{
	return Grid.X >= 0 && Grid.X < Width && Grid.Y >= 0 && Grid.Y < Height;
}

TCHAR ASlideMazeGameManager::GetTileCharAt(FIntPoint Grid) const
{
	if (!IsInsideGrid(Grid) || !MapRows.IsValidIndex(Grid.Y) || MapRows[Grid.Y].Len() <= Grid.X)
	{
		return TEXT('#');
	}

	return MapRows[Grid.Y][Grid.X];
}

void ASlideMazeGameManager::SetTileCharAt(FIntPoint Grid, TCHAR NewChar)
{
	if (!IsInsideGrid(Grid) || !MapRows.IsValidIndex(Grid.Y) || MapRows[Grid.Y].Len() <= Grid.X)
	{
		return;
	}

	MapRows[Grid.Y][Grid.X] = NewChar;
}

ESlideMazeTileType ASlideMazeGameManager::GetTileTypeAt(FIntPoint Grid) const
{
	const TCHAR Symbol = GetTileCharAt(Grid);
	if (Symbol == TEXT('#'))
	{
		return ESlideMazeTileType::Wall;
	}
	if (Symbol == TEXT('X'))
	{
		return ESlideMazeTileType::Breakable;
	}
	if (Symbol == TEXT('S'))
	{
		return ESlideMazeTileType::Start;
	}
	if (Symbol == TEXT('G'))
	{
		return ESlideMazeTileType::Goal;
	}
	if (IsTeleportEntrance(Symbol))
	{
		return ESlideMazeTileType::TeleportIn;
	}
	if (IsTeleportExit(Symbol))
	{
		return ESlideMazeTileType::TeleportOut;
	}
	return ESlideMazeTileType::Empty;
}

FIntPoint ASlideMazeGameManager::DirectionToDelta(ESlideMazeDirection Direction) const
{
	switch (Direction)
	{
	case ESlideMazeDirection::Up:
		return FIntPoint(0, -1);
	case ESlideMazeDirection::Down:
		return FIntPoint(0, 1);
	case ESlideMazeDirection::Left:
		return FIntPoint(-1, 0);
	case ESlideMazeDirection::Right:
		return FIntPoint(1, 0);
	default:
		return FIntPoint::ZeroValue;
	}
}

bool ASlideMazeGameManager::FindTeleportExit(TCHAR EntranceSymbol, FIntPoint& OutExitGrid) const
{
	const TCHAR ExitSymbol = FChar::ToLower(EntranceSymbol);
	for (int32 Row = 0; Row < Height; ++Row)
	{
		for (int32 Col = 0; Col < Width; ++Col)
		{
			if (GetTileCharAt(FIntPoint(Col, Row)) == ExitSymbol)
			{
				OutExitGrid = FIntPoint(Col, Row);
				return true;
			}
		}
	}

	return false;
}

void ASlideMazeGameManager::ConfigureFloorInstanceComponent()
{
	UClass* VisualTileClass = TileClass ? TileClass.Get() : ASlideMazeTileActor::StaticClass();
	const ASlideMazeTileActor* TileDefaults = VisualTileClass ? VisualTileClass->GetDefaultObject<ASlideMazeTileActor>() : nullptr;

	UStaticMesh* TileMesh = TileDefaults ? TileDefaults->GetTileMesh() : nullptr;
	FVector MeshRelativeScale = TileDefaults ? TileDefaults->GetTileMeshRelativeScale() : FVector(1.0f, 1.0f, 0.05f);
	if (!TileMesh)
	{
		TileMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	if (!EmptyTileInstances)
	{
		return;
	}

	EmptyTileInstances->ClearInstances();
	EmptyTileInstances->SetStaticMesh(TileMesh);
	EmptyTileInstances->SetRelativeScale3D(MeshRelativeScale);
	EmptyTileInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (TileDefaults)
	{
		if (UMaterialInterface* Material = TileDefaults->GetMaterialForTileType(ESlideMazeTileType::Empty))
		{
			EmptyTileInstances->SetMaterial(0, Material);
		}
	}
}

FTransform ASlideMazeGameManager::MakeFloorInstanceTransform(FIntPoint Grid) const
{
	const float LocalX = (static_cast<float>(Grid.X) - static_cast<float>(Width) * 0.5f + 0.5f) * TileSize;
	const float LocalY = (static_cast<float>(Grid.Y) - static_cast<float>(Height) * 0.5f + 0.5f) * TileSize;
	return FTransform(FRotator::ZeroRotator, FVector(LocalX, LocalY, 0.0f), FVector(TileSize / 100.0f, TileSize / 100.0f, 1.0f));
}

TCHAR ASlideMazeGameManager::EmptyChar()
{
	return TEXT("□")[0];
}

bool ASlideMazeGameManager::IsTeleportEntrance(TCHAR Symbol)
{
	return Symbol == TEXT('A') || Symbol == TEXT('B') || Symbol == TEXT('C');
}

bool ASlideMazeGameManager::IsTeleportExit(TCHAR Symbol)
{
	return Symbol == TEXT('a') || Symbol == TEXT('b') || Symbol == TEXT('c');
}
