#include "Breakout/BreakoutGameManager.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Breakout/BreakoutBall.h"
#include "Breakout/BreakoutBrick.h"
#include "Breakout/BreakoutCaptureActor.h"
#include "Breakout/BreakoutHUDWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABreakoutGameManager::ABreakoutGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABreakoutGameManager::BeginPlay()
{
	Super::BeginPlay();

	FindOrCreateHUDWidget();
	SetWidgetVisible(false);

	if (BallToControl)
	{
		BallToControl->SetGameManager(this);
		BallToControl->StopBall();
	}

	if (bAutoStartOnBeginPlay)
	{
		StartMiniGame();
	}
}

void ABreakoutGameManager::StartMiniGame()
{
	bMiniGameActive = true;
	bIsGameOver = false;
	bIsGameCleared = false;

	FindOrCreateHUDWidget();
	SetWidgetVisible(true);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	if (BallToControl)
	{
		BallToControl->SetGameManager(this);
		BallToControl->ResetBall();
		BallToControl->LaunchBall();
	}

	StartBrickDropTimer();

	OnMiniGameStarted.Broadcast();
}

void ABreakoutGameManager::StopMiniGame()
{
	bMiniGameActive = false;
	StopBrickDropTimer();

	if (BallToControl)
	{
		BallToControl->StopBall();
	}

	SetWidgetVisible(false);
}

void ABreakoutGameManager::RestartBreakoutRound()
{
	bIsGameOver = false;
	bIsGameCleared = false;
	bMiniGameActive = true;
	StopBrickDropTimer();

	if (BallToControl)
	{
		BallToControl->SetGameManager(this);
		BallToControl->ResetBall();
		BallToControl->LaunchBall();
	}

	StartBrickDropTimer();
}

void ABreakoutGameManager::HandleBallLost(ABreakoutBall* LostBall)
{
	if (!bMiniGameActive || bIsGameOver)
	{
		return;
	}

	if (LostBall)
	{
		LostBall->StopBall();
	}

	TriggerGameOver();
}

void ABreakoutGameManager::HandleBrickReachedKillZone(ABreakoutBrick* Brick)
{
	if (!bMiniGameActive || bIsGameOver || !IsValid(Brick))
	{
		return;
	}

	TriggerGameOver();
}

void ABreakoutGameManager::HandleBrickTouchedPaddle(ABreakoutBrick* Brick)
{
	if (!bMiniGameActive || bIsGameOver || !IsValid(Brick))
	{
		return;
	}

	TriggerGameOver();
}

void ABreakoutGameManager::TriggerGameOver()
{
	if (bIsGameOver)
	{
		return;
	}

	bIsGameOver = true;
	bMiniGameActive = false;
	StopBrickDropTimer();

	if (BallToControl)
	{
		BallToControl->StopBall();
	}

	OnGameOver.Broadcast();
}

void ABreakoutGameManager::SpawnBrickGrid(int32 RowCount, int32 ColumnCount, FVector StartLocation)
{
	if (!BrickClass || RowCount <= 0 || ColumnCount <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ABreakoutBrick* DefaultBrick = BrickClass->GetDefaultObject<ABreakoutBrick>();
	const FVector BrickSize = DefaultBrick ? DefaultBrick->GetBrickSize() : FVector::ZeroVector;
	if (BrickSize.IsNearlyZero())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
	{
		for (int32 ColumnIndex = 0; ColumnIndex < ColumnCount; ++ColumnIndex)
		{
			const FVector SpawnLocation = StartLocation + FVector(
				0.0f,
				BrickSize.Y * ColumnIndex,
				-BrickSize.Z * RowIndex);

			if (ABreakoutBrick* SpawnedBrick = World->SpawnActor<ABreakoutBrick>(
				BrickClass,
				SpawnLocation,
				FRotator::ZeroRotator,
				SpawnParameters))
			{
				SpawnedBrick->OnDestroyed.AddDynamic(this, &ABreakoutGameManager::HandleBrickDestroyed);
				SpawnedBricks.Add(SpawnedBrick);
			}
		}
	}
}

void ABreakoutGameManager::ClearSpawnedBricks()
{
	const bool bWasSuppressingBrickDestroyedNotifications = bSuppressBrickDestroyedNotifications;
	bSuppressBrickDestroyedNotifications = true;

	for (ABreakoutBrick* SpawnedBrick : SpawnedBricks)
	{
		if (IsValid(SpawnedBrick))
		{
			SpawnedBrick->Destroy();
		}
	}

	SpawnedBricks.Reset();
	bSuppressBrickDestroyedNotifications = bWasSuppressingBrickDestroyedNotifications;
}

void ABreakoutGameManager::StartBrickDropTimer()
{
	if (!bEnableTimedBrickDrop || BrickDropInterval <= 0.0f)
	{
		return;
	}

	ScheduleNextBrickDrop(BrickDropInterval);
}

void ABreakoutGameManager::StopBrickDropTimer()
{
	GetWorldTimerManager().ClearTimer(BrickDropTimerHandle);
}

void ABreakoutGameManager::ScheduleNextBrickDrop(float DelaySeconds)
{
	if (!bEnableTimedBrickDrop || DelaySeconds <= 0.0f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		BrickDropTimerHandle,
		this,
		&ABreakoutGameManager::MoveBricksDownOneRow,
		DelaySeconds,
		false);
}

void ABreakoutGameManager::MoveBricksDownOneRow()
{
	if (!bMiniGameActive || bIsGameOver)
	{
		return;
	}

	RemoveInvalidBrickReferences();

	const float DropDistance = ResolveBrickDropDistance();
	if (DropDistance <= 0.0f || SpawnedBricks.Num() == 0)
	{
		ScheduleNextBrickDrop(BrickDropInterval);
		return;
	}

	if (bSkipBrickDropWhenBallWouldBeTrapped && WouldBrickDropTrapBall(DropDistance))
	{
		ScheduleNextBrickDrop(BrickDropRetryInterval);
		return;
	}

	const FVector DropOffset(0.0f, 0.0f, -DropDistance);
	for (ABreakoutBrick* SpawnedBrick : SpawnedBricks)
	{
		if (IsValid(SpawnedBrick))
		{
			SpawnedBrick->AddActorWorldOffset(DropOffset, false);
		}
	}

	ScheduleNextBrickDrop(BrickDropInterval);
}

float ABreakoutGameManager::ResolveBrickDropDistance() const
{
	if (BrickDropDistance > 0.0f)
	{
		return BrickDropDistance;
	}

	const ABreakoutBrick* DefaultBrick = BrickClass ? BrickClass->GetDefaultObject<ABreakoutBrick>() : nullptr;
	return DefaultBrick ? DefaultBrick->GetBrickSize().Z : 0.0f;
}

bool ABreakoutGameManager::WouldBrickDropTrapBall(float DropDistance) const
{
	if (!IsValid(BallToControl) || DropDistance <= 0.0f)
	{
		return false;
	}

	const FBox BallBounds = BallToControl->GetComponentsBoundingBox(true).ExpandBy(BrickBallSafetyPadding);
	if (!BallBounds.IsValid)
	{
		return false;
	}

	const FVector DropOffset(0.0f, 0.0f, -DropDistance);
	for (const ABreakoutBrick* SpawnedBrick : SpawnedBricks)
	{
		if (!IsValid(SpawnedBrick))
		{
			continue;
		}

		FBox BrickPathBounds = SpawnedBrick->GetComponentsBoundingBox(true).ExpandBy(BrickBallSafetyPadding);
		if (!BrickPathBounds.IsValid)
		{
			continue;
		}

		BrickPathBounds += BrickPathBounds.ShiftBy(DropOffset);
		if (BrickPathBounds.Intersect(BallBounds))
		{
			return true;
		}
	}

	return false;
}

void ABreakoutGameManager::RemoveInvalidBrickReferences()
{
	for (int32 Index = SpawnedBricks.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(SpawnedBricks[Index]))
		{
			SpawnedBricks.RemoveAtSwap(Index);
		}
	}
}

void ABreakoutGameManager::HandleBrickDestroyed(AActor* DestroyedActor)
{
	if (bSuppressBrickDestroyedNotifications)
	{
		return;
	}

	OnBrickDestroyed.Broadcast(DestroyedActor);

	SpawnedBricks.RemoveAll([DestroyedActor](const TObjectPtr<ABreakoutBrick>& SpawnedBrick)
	{
		return SpawnedBrick.Get() == DestroyedActor;
	});
	RemoveInvalidBrickReferences();

	if (SpawnedBricks.Num() == 0)
	{
		HandleMiniGameCleared();
	}
}

void ABreakoutGameManager::HandleMiniGameCleared()
{
	if (!bMiniGameActive || bIsGameOver || bIsGameCleared)
	{
		return;
	}

	bIsGameCleared = true;
	bMiniGameActive = false;
	StopBrickDropTimer();

	if (BallToControl)
	{
		BallToControl->StopBall();
	}

	OnMiniGameCleared.Broadcast();
}

void ABreakoutGameManager::FindOrCreateHUDWidget()
{
	if (!HUDWidgetClass)
	{
		return;
	}

	if (ActiveHUDWidget)
	{
		ConfigureWidgetMaterial();
		return;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, HUDWidgetClass, false);

	for (UUserWidget* FoundWidget : FoundWidgets)
	{
		if (UBreakoutHUDWidget* ExistingWidget = Cast<UBreakoutHUDWidget>(FoundWidget))
		{
			ActiveHUDWidget = ExistingWidget;
			break;
		}
	}

	if (!ActiveHUDWidget && bCreateWidgetIfMissing)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			ActiveHUDWidget = CreateWidget<UBreakoutHUDWidget>(PlayerController, HUDWidgetClass);
			if (ActiveHUDWidget)
			{
				ActiveHUDWidget->AddToViewport(WidgetZOrder);
			}
		}
	}

	ConfigureWidgetMaterial();
}

void ABreakoutGameManager::SetWidgetVisible(bool bVisible)
{
	if (!ActiveHUDWidget)
	{
		return;
	}

	ActiveHUDWidget->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

UTextureRenderTarget2D* ABreakoutGameManager::ResolveRenderTarget() const
{
	return CaptureActor ? CaptureActor->GetRenderTarget() : nullptr;
}

void ABreakoutGameManager::ConfigureWidgetMaterial() const
{
	if (!ActiveHUDWidget)
	{
		return;
	}

	ActiveHUDWidget->InitializeCaptureMaterial(CaptureUIMaterial, ResolveRenderTarget());
}
