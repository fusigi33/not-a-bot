#include "DominoMiniGame/DominoMiniGameManager.h"

#include "DominoMiniGame/DominoBlockActor.h"
#include "DominoMiniGame/DominoBoardActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDominoMiniGame, Log, All);

ADominoMiniGameManager::ADominoMiniGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADominoMiniGameManager::BeginPlay()
{
	Super::BeginPlay();
}

void ADominoMiniGameManager::BeginRound()
{
	ResetRound();

	EnsureEndpointDominoes();

	if (StartDomino)
	{
		StartDomino->SetPlacementMode(true);
	}

	if (GoalDomino)
	{
		GoalDomino->SetPlacementMode(true);
	}

	CurrentPlacementRotation = RoundData.DefaultDominoRotation;
	CurrentState = EDominoMiniGameState::Placement;
}

void ADominoMiniGameManager::ResetRound()
{
	GetWorldTimerManager().ClearTimer(GoalCheckTimerHandle);
	GetWorldTimerManager().ClearTimer(SimulationTimeoutTimerHandle);

	CancelPreviewDomino();

	for (ADominoBlockActor* Domino : PlacedDominoes)
	{
		if (IsValid(Domino))
		{
			Domino->Destroy();
		}
	}
	PlacedDominoes.Reset();

	if (StartDomino)
	{
		StartDomino->ResetDominoTransform();
		StartDomino->SetPlacementMode(true);
	}

	if (GoalDomino)
	{
		GoalDomino->ResetDominoTransform();
		GoalDomino->SetPlacementMode(true);
	}

	bPreviewPlacementValid = false;
	CurrentState = EDominoMiniGameState::None;
}

bool ADominoMiniGameManager::LoadRoundDataFromDataTable(UDataTable* DataTable, FName RowName)
{
	if (!DataTable)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("RoundData를 불러올 DataTable이 없습니다."));
		return false;
	}

	if (RowName.IsNone())
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("RoundData를 불러올 RowName이 비어 있습니다."));
		return false;
	}

	if (DataTable->GetRowStruct() != FDominoRoundData::StaticStruct())
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("DataTable의 RowStruct가 FDominoRoundData가 아닙니다."));
		return false;
	}

	const FDominoRoundData* FoundRoundData = DataTable->FindRow<FDominoRoundData>(RowName, TEXT("LoadRoundDataFromDataTable"));
	if (!FoundRoundData)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("DataTable에서 RoundData Row를 찾을 수 없습니다. RowName: %s"), *RowName.ToString());
		return false;
	}

	RoundData = *FoundRoundData;
	CurrentPlacementRotation = RoundData.DefaultDominoRotation;
	return true;
}

bool ADominoMiniGameManager::RequestPreviewDominoAtWorldLocation(const FVector& WorldLocation)
{
	if (CurrentState != EDominoMiniGameState::Placement)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("Preview request failed: CurrentState is not Placement. CurrentState=%d"), static_cast<int32>(CurrentState));
		return false;
	}

	if (!RoundData.DominoClass)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("RoundData.DominoClass가 설정되지 않았습니다."));
		return false;
	}

	FVector PlacementLocation = WorldLocation;
	if (BoardActor)
	{
		PlacementLocation = BoardActor->SnapLocationToBoard(WorldLocation);
	}
	else
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("Preview request has no BoardActor. Using raw world location: %s"), *WorldLocation.ToString());
	}

	PlacementLocation = GetDominoActorLocationFromBoardLocation(PlacementLocation);

	if (!PreviewDomino)
	{
		PreviewDomino = SpawnDomino(PlacementLocation, CurrentPlacementRotation, true);
	}

	if (!PreviewDomino)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("Preview request failed: preview domino could not be spawned. PlacementLocation=%s"), *PlacementLocation.ToString());
		return false;
	}

	PreviewDomino->SetActorLocationAndRotation(PlacementLocation, CurrentPlacementRotation, false, nullptr, ETeleportType::TeleportPhysics);
	bPreviewPlacementValid = CanPlaceDominoAt(PlacementLocation, CurrentPlacementRotation);
	PreviewDomino->SetPreviewValid(bPreviewPlacementValid);

	UE_LOG(LogDominoMiniGame, Warning, TEXT("Preview updated. RawWorldLocation=%s, PlacementLocation=%s, bPreviewPlacementValid=%s"),
		*WorldLocation.ToString(),
		*PlacementLocation.ToString(),
		bPreviewPlacementValid ? TEXT("true") : TEXT("false"));

	return bPreviewPlacementValid;
}

bool ADominoMiniGameManager::ConfirmPlacePreviewDomino()
{
	if (CurrentState != EDominoMiniGameState::Placement || !PreviewDomino)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("Confirm place failed: invalid state or missing preview. CurrentState=%d, PreviewDomino=%s"),
			static_cast<int32>(CurrentState),
			PreviewDomino ? TEXT("Valid") : TEXT("None"));
		return false;
	}

	const FVector Location = PreviewDomino->GetActorLocation();
	const FRotator Rotation = PreviewDomino->GetActorRotation();

	if (!bPreviewPlacementValid || !CanPlaceDominoAt(Location, Rotation))
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("Confirm place failed: placement invalid. Location=%s, bPreviewPlacementValid=%s"),
			*Location.ToString(),
			bPreviewPlacementValid ? TEXT("true") : TEXT("false"));
		CancelPreviewDomino();
		return false;
	}

	ADominoBlockActor* PlacedDomino = PreviewDomino;
	PreviewDomino = nullptr;

	const FVector DropLocation = Location + FVector::UpVector * PlacementDropHeight;

	PlacedDomino->SetAsPreview(false);
	ConfigurePlacedDominoPhysics(PlacedDomino, true);
	PlacedDomino->SetActorLocationAndRotation(DropLocation, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	PlacedDomino->SetPhysicsEnabled(true);
	PlacedDominoes.Add(PlacedDomino);

	bPreviewPlacementValid = false;
	return true;
}

void ADominoMiniGameManager::CancelPreviewDomino()
{
	if (PreviewDomino)
	{
		PreviewDomino->Destroy();
		PreviewDomino = nullptr;
	}

	bPreviewPlacementValid = false;
}

bool ADominoMiniGameManager::CanPlaceDominoAt(const FVector& WorldLocation, FRotator Rotation) const
{
	if (!GetWorld())
	{
		return false;
	}

	if (BoardActor && !BoardActor->IsInsideBoard(WorldLocation))
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("CanPlaceDominoAt failed: location is outside BoardActor. Location=%s, BoardActor=%s, BoardSize=%s, BoardPlaneZ=%f"),
			*WorldLocation.ToString(),
			*BoardActor->GetName(),
			*BoardActor->BoardSize.ToString(),
			BoardActor->BoardPlaneZ);
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DominoPlacementOverlap), false);
	QueryParams.AddIgnoredActor(this);
	if (BoardActor)
	{
		QueryParams.AddIgnoredActor(BoardActor);
	}

	if (PreviewDomino)
	{
		QueryParams.AddIgnoredActor(PreviewDomino);
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(PlacementHalfExtent);
	const FQuat RotationQuat = Rotation.Quaternion();

	const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		WorldLocation,
		RotationQuat,
		PlacementTraceChannel,
		BoxShape,
		QueryParams
	);

	if (!bHasOverlap)
	{
		return true;
	}

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || HitActor == this || HitActor == PreviewDomino)
		{
			continue;
		}

		// 시작/목표/기존 도미노/장애물 등 쿼리에 잡히는 블로킹 액터와 겹치면 배치 불가입니다.
		UE_LOG(LogDominoMiniGame, Warning, TEXT("CanPlaceDominoAt failed: overlap with %s at Location=%s"),
			*HitActor->GetName(),
			*WorldLocation.ToString());
		return false;
	}

	return true;
}

void ADominoMiniGameManager::StartSimulation()
{
	if (CurrentState != EDominoMiniGameState::Placement)
	{
		return;
	}

	CancelPreviewDomino();
	EnsureEndpointDominoes();

	if (!StartDomino || !GoalDomino)
	{
		UE_LOG(LogDominoMiniGame, Warning, TEXT("시작 도미노 또는 목표 도미노가 없어 시뮬레이션을 시작할 수 없습니다."));
		HandleFailure();
		return;
	}

	CurrentState = EDominoMiniGameState::Simulating;

	StartDomino->SetPhysicsEnabled(true);
	GoalDomino->SetPhysicsEnabled(true);

	for (ADominoBlockActor* Domino : PlacedDominoes)
	{
		if (IsValid(Domino))
		{
			ConfigurePlacedDominoPhysics(Domino, false);
			Domino->SetPhysicsEnabled(true);
		}
	}

	StartDomino->ApplyInitialFallImpulse(RoundData.InitialFallDirection);

	GetWorldTimerManager().SetTimer(GoalCheckTimerHandle, this, &ADominoMiniGameManager::CheckGoalDominoFallen, GoalCheckInterval, true);
	GetWorldTimerManager().SetTimer(SimulationTimeoutTimerHandle, this, &ADominoMiniGameManager::HandleSimulationTimeout, RoundData.SimulationTimeLimit, false);
}

void ADominoMiniGameManager::CheckGoalDominoFallen()
{
	if (CurrentState != EDominoMiniGameState::Simulating || !GoalDomino)
	{
		return;
	}

	if (GoalDomino->IsFallen())
	{
		HandleSuccess();
	}
}

void ADominoMiniGameManager::HandleSuccess()
{
	if (CurrentState == EDominoMiniGameState::Success || CurrentState == EDominoMiniGameState::Failed)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(GoalCheckTimerHandle);
	GetWorldTimerManager().ClearTimer(SimulationTimeoutTimerHandle);

	CurrentState = EDominoMiniGameState::Success;
	OnDominoMiniGameSucceeded();
}

void ADominoMiniGameManager::HandleFailure()
{
	if (CurrentState == EDominoMiniGameState::Success || CurrentState == EDominoMiniGameState::Failed)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(GoalCheckTimerHandle);
	GetWorldTimerManager().ClearTimer(SimulationTimeoutTimerHandle);

	CurrentState = EDominoMiniGameState::Failed;
	OnDominoMiniGameFailed();
}

void ADominoMiniGameManager::OnDominoMiniGameSucceeded()
{
	OnMiniGameSucceeded.Broadcast();
	BP_OnMiniGameSucceeded();
}

void ADominoMiniGameManager::OnDominoMiniGameFailed()
{
	OnMiniGameFailed.Broadcast();
	BP_OnMiniGameFailed();
}

void ADominoMiniGameManager::EnsureEndpointDominoes()
{
	if (!RoundData.DominoClass)
	{
		return;
	}

	if (!StartDomino)
	{
		StartDomino = SpawnDomino(RoundData.StartDominoLocation, RoundData.DefaultDominoRotation, false);
	}

	if (!GoalDomino)
	{
		GoalDomino = SpawnDomino(RoundData.GoalDominoLocation, RoundData.DefaultDominoRotation, false);
	}
}

ADominoBlockActor* ADominoMiniGameManager::SpawnDomino(const FVector& Location, const FRotator& Rotation, bool bPreview) const
{
	if (!GetWorld() || !RoundData.DominoClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = const_cast<ADominoMiniGameManager*>(this);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADominoBlockActor* Domino = GetWorld()->SpawnActor<ADominoBlockActor>(RoundData.DominoClass, Location, Rotation, SpawnParams);
	if (Domino)
	{
		Domino->PlacementHalfExtent = PlacementHalfExtent;
		Domino->SetAsPreview(bPreview);
		Domino->SetPlacementMode(true);
	}

	return Domino;
}

FVector ADominoMiniGameManager::GetDominoActorLocationFromBoardLocation(const FVector& BoardLocation) const
{
	FVector ActorLocation = BoardLocation;

	if (bAlignDominoCenterToCursor)
	{
		ActorLocation.Z -= PlacementHalfExtent.Z;
	}

	return bPlaceDominoPivotAboveBoard ? AdjustDominoLocationAboveBoard(ActorLocation) : ActorLocation;
}

FVector ADominoMiniGameManager::AdjustDominoLocationAboveBoard(const FVector& ActorLocation) const
{
	if (!BoardActor)
	{
		return ActorLocation;
	}

	const FBox BoardComponentBounds = BoardActor->GetComponentsBoundingBox(true);
	if (!BoardComponentBounds.IsValid)
	{
		return ActorLocation;
	}

	const FTransform BoardTransform = BoardActor->GetActorTransform();
	float MaxBoardLocalY = -FLT_MAX;
	for (int32 XIndex = 0; XIndex < 2; ++XIndex)
	{
		for (int32 YIndex = 0; YIndex < 2; ++YIndex)
		{
			for (int32 ZIndex = 0; ZIndex < 2; ++ZIndex)
			{
				const FVector WorldCorner(
					XIndex == 0 ? BoardComponentBounds.Min.X : BoardComponentBounds.Max.X,
					YIndex == 0 ? BoardComponentBounds.Min.Y : BoardComponentBounds.Max.Y,
					ZIndex == 0 ? BoardComponentBounds.Min.Z : BoardComponentBounds.Max.Z
				);
				MaxBoardLocalY = FMath::Max(MaxBoardLocalY, BoardTransform.InverseTransformPosition(WorldCorner).Y);
			}
		}
	}

	FVector AdjustedLocalLocation = BoardTransform.InverseTransformPosition(ActorLocation);
	const float TargetCenterLocalY = MaxBoardLocalY + PlacementHalfExtent.Y + PlacementSurfaceClearance;

	if (AdjustedLocalLocation.Y < TargetCenterLocalY)
	{
		AdjustedLocalLocation.Y = TargetCenterLocalY;
	}

	return BoardTransform.TransformPosition(AdjustedLocalLocation);
}

void ADominoMiniGameManager::ConfigurePlacedDominoPhysics(ADominoBlockActor* Domino, bool bUsePlacementStabilization) const
{
	if (!Domino || !Domino->MeshComponent)
	{
		return;
	}

	Domino->MeshComponent->BodyInstance.COMNudge = bUsePlacementStabilization ? PlacementCOMNudge : FVector::ZeroVector;
	Domino->MeshComponent->SetAngularDamping(bUsePlacementStabilization ? PlacementAngularDamping : 0.0f);
	Domino->MeshComponent->BodyInstance.UpdateMassProperties();
}

void ADominoMiniGameManager::HandleSimulationTimeout()
{
	if (CurrentState == EDominoMiniGameState::Simulating)
	{
		HandleFailure();
	}
}
