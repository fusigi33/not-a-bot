#include "ShellGame/ShellGameManager.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ShellGame/ShellBoard.h"
#include "ShellGame/ShellCaptureRig.h"
#include "ShellGame/ShellCup.h"
#include "ShellGame/ShellDistractorBall.h"
#include "ShellGame/ShellGameWidget.h"
#include "ShellGame/ShellObstacleMover.h"
#include "ShellGame/ShellSelectionResolver.h"
#include "TimerManager.h"

AShellGameManager::AShellGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShellGameManager::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	SelectionResolver = NewObject<UShellSelectionResolver>(this);

	if (Board)
	{
		Board->OnIntroFinished.AddDynamic(this, &AShellGameManager::HandleBoardIntroFinished);
		Board->OnShuffleFinished.AddDynamic(this, &AShellGameManager::HandleBoardShuffleFinished);
		Board->OnRevealFinished.AddDynamic(this, &AShellGameManager::HandleBoardRevealFinished);
	}

	FindAndBindExistingWidget();
	ConfigurePlayerInput();

	if (bAutoStartOnBeginPlay)
	{
		StartShellGame();
	}
}

void AShellGameManager::StartShellGame()
{
	if (!Board)
	{
		UE_LOG(LogTemp, Warning, TEXT("AShellGameManager::StartShellGame - Board reference is missing."));
		return;
	}
	
	// 상태를 리셋 중으로 변경하여 UI 등에서 입력을 막도록 일관성 유지
	TransitionToState(EShellGameState::Resetting);

	GetWorldTimerManager().ClearTimer(AutoStartTimerHandle);
	
	// [핵심 수정] 타이머(카운트다운) 시작 전에 보드와 컵을 즉시 초기 상태로 되돌립니다.
	PrepareRoundForRestart();
	
	GetWorldTimerManager().ClearTimer(PendingStartTimerHandle);
	GetWorldTimerManager().SetTimer(PendingStartTimerHandle, this, &AShellGameManager::ExecuteStartShellGame, AutoStartDelay, false);
}

void AShellGameManager::RequestResetRound()
{
	TransitionToState(EShellGameState::Resetting);
	GetWorldTimerManager().ClearTimer(AutoRestartTimerHandle);
	GetWorldTimerManager().ClearTimer(PendingResetTimerHandle);
	PrepareRoundForRestart();
	GetWorldTimerManager().SetTimer(PendingResetTimerHandle, this, &AShellGameManager::ExecuteResetRound, AutoRestartDelay, false);
}

void AShellGameManager::SubmitCupSelection(AShellCup* SelectedCup)
{
	if (!CanAcceptSelection() || !Board || !SelectedCup)
	{
		return;
	}

	TransitionToState(EShellGameState::RevealResult);
	SetObstacleActivation(false);
	Board->StartRevealForSelection(SelectedCup);
}

void AShellGameManager::TransitionToState(EShellGameState NewState)
{
	CurrentState = NewState;

	if (ShellGameWidget)
	{
		ShellGameWidget->SetBoardInputEnabled(CurrentState == EShellGameState::AwaitSelection);
	}

	if (Board)
	{
		Board->SetCupSelectionEnabled(CurrentState == EShellGameState::AwaitSelection);
	}

	if (CurrentState != EShellGameState::AwaitSelection)
	{
		UpdateHoveredCup(nullptr);
	}
}

void AShellGameManager::HandleBoardImageClicked(FVector2D UV)
{
	if (!CanAcceptSelection() || !SelectionResolver || !CaptureRig)
	{
		return;
	}

	USceneCaptureComponent2D* SceneCapture = CaptureRig->GetSceneCapture();
	UTextureRenderTarget2D* RenderTarget = CaptureRig->GetRenderTarget();
	if (!SceneCapture || !RenderTarget)
	{
		return;
	}

	const FShellSelectionResult SelectionResult = SelectionResolver->ResolveSelectionFromCaptureUV(
		this,
		SceneCapture,
		RenderTarget,
		UV,
		SelectionTraceDistance,
		SelectionTraceChannel
	);

	if (SelectionResult.bValidSelection && SelectionResult.SelectedCup)
	{
		SubmitCupSelection(SelectionResult.SelectedCup);
	}
}

void AShellGameManager::HandleBoardImageHovered(FVector2D UV)
{
	if (!CanAcceptSelection() || !SelectionResolver || !CaptureRig)
	{
		UpdateHoveredCup(nullptr);
		return;
	}

	USceneCaptureComponent2D* SceneCapture = CaptureRig->GetSceneCapture();
	UTextureRenderTarget2D* RenderTarget = CaptureRig->GetRenderTarget();
	if (!SceneCapture || !RenderTarget)
	{
		UpdateHoveredCup(nullptr);
		return;
	}

	UpdateHoveredCup(SelectionResolver->ResolveCupFromCaptureUV(
		this,
		SceneCapture,
		RenderTarget,
		UV,
		SelectionTraceDistance,
		SelectionTraceChannel
	));
}

void AShellGameManager::HandleBoardHoverEnded()
{
	UpdateHoveredCup(nullptr);
}

void AShellGameManager::HandleBoardIntroFinished()
{
	TransitionToState(EShellGameState::Shuffling);
	SetObstacleActivation(true);

	if (Board)
	{
		Board->StartShuffleSequence();
	}
}

void AShellGameManager::HandleBoardShuffleFinished()
{
	TransitionToState(EShellGameState::AwaitSelection);
	SetObstacleActivation(false);
}

void AShellGameManager::HandleBoardRevealFinished(const FShellSelectionResult& Result)
{
	UE_LOG(LogTemp, Log, TEXT("Shell Game Result: Correct=%s SelectedSlot=%d WinningSlot=%d"),
		Result.bIsCorrect ? TEXT("true") : TEXT("false"),
		Result.SelectedSlotIndex,
		Result.WinningSlotIndex);

	if (bAutoRestartAfterReveal)
	{
		RequestResetRound();
	}
	else
	{
		TransitionToState(EShellGameState::Resetting);
	}
}

void AShellGameManager::SetObstacleActivation(bool bActive)
{
	for (AShellObstacleMover* Obstacle : Obstacles)
	{
		if (!Obstacle)
		{
			continue;
		}

		if (bActive)
		{
			Obstacle->ActivateObstacleMovement();
		}
		else
		{
			Obstacle->DeactivateObstacleMovement();
		}
	}
}

void AShellGameManager::FindAndBindExistingWidget()
{
	if (!WidgetClass || !PlayerController)
	{
		return;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, WidgetClass, false);

	for (UUserWidget* FoundWidget : FoundWidgets)
	{
		if (UShellGameWidget* ExistingWidget = Cast<UShellGameWidget>(FoundWidget))
		{
			ShellGameWidget = ExistingWidget;
			break;
		}
	}

	if (!ShellGameWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("AShellGameManager::FindAndBindExistingWidget - No existing widget of the configured class was found."));
		return;
	}

	ShellGameWidget->OnBoardImageClicked.AddUniqueDynamic(this, &AShellGameManager::HandleBoardImageClicked);
	ShellGameWidget->OnBoardImageHovered.AddUniqueDynamic(this, &AShellGameManager::HandleBoardImageHovered);
	ShellGameWidget->OnBoardHoverEnded.AddUniqueDynamic(this, &AShellGameManager::HandleBoardHoverEnded);
}

void AShellGameManager::ConfigurePlayerInput() const
{
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}

bool AShellGameManager::CanAcceptSelection() const
{
	return CurrentState == EShellGameState::AwaitSelection;
}

void AShellGameManager::ExecuteStartShellGame()
{
	StartRoundInternal();
}

void AShellGameManager::ExecuteResetRound()
{
	BeginRoundAfterDelay();
}

void AShellGameManager::PrepareRoundForRestart()
{
	if (!Board)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PendingStartTimerHandle);
	GetWorldTimerManager().ClearTimer(PendingResetTimerHandle);
	ClearDistractorBalls();
	Board->ResetBoardForNewRound();
	SetObstacleActivation(false);
}

void AShellGameManager::BeginRoundAfterDelay()
{
	if (!Board)
	{
		return;
	}

	TransitionToState(EShellGameState::IntroDrop);
	SpawnDistractorBalls();
	Board->StartIntroDrop();
}

void AShellGameManager::UpdateHoveredCup(AShellCup* NewHoveredCup)
{
	if (HoveredCup == NewHoveredCup)
	{
		return;
	}

	if (HoveredCup)
	{
		HoveredCup->SetHoverHighlighted(false);
	}

	HoveredCup = NewHoveredCup;

	if (HoveredCup && CanAcceptSelection())
	{
		HoveredCup->SetHoverHighlighted(true);
	}
}

void AShellGameManager::StartRoundInternal()
{
	if (!Board)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoRestartTimerHandle);
	GetWorldTimerManager().ClearTimer(PendingStartTimerHandle);
	GetWorldTimerManager().ClearTimer(PendingResetTimerHandle);
	// ClearDistractorBalls();
	// Board->ResetBoardForNewRound();
	TransitionToState(EShellGameState::IntroDrop);
	SetObstacleActivation(false);
	SpawnDistractorBalls();
	Board->StartIntroDrop();
}

void AShellGameManager::SpawnDistractorBalls()
{
	if (!bSpawnDistractorBallsOnRoundStart || DistractorBallCount <= 0 || !GetWorld())
	{
		return;
	}

	UClass* SpawnClass = DistractorBallClass ? DistractorBallClass.Get() : AShellDistractorBall::StaticClass();
	if (!SpawnClass)
	{
		return;
	}

	const FVector BaseLocation = Board
		? Board->GetActorLocation() + DistractorSpawnOffset
		: GetActorLocation() + DistractorSpawnOffset;
	const float ConeHalfAngleRadians = FMath::DegreesToRadians(DistractorConeHalfAngleDegrees);
	const float MinSpeed = FMath::Min(DistractorInitialSpeedMin, DistractorInitialSpeedMax);
	const float MaxSpeed = FMath::Max(DistractorInitialSpeedMin, DistractorInitialSpeedMax);
	const float MinScale = FMath::Min(DistractorBallScaleMin, DistractorBallScaleMax);
	const float MaxScale = FMath::Max(DistractorBallScaleMin, DistractorBallScaleMax);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveDistractorBalls.Reserve(DistractorBallCount);

	for (int32 BallIndex = 0; BallIndex < DistractorBallCount; ++BallIndex)
	{
		const FVector2D SpawnOffset2D = FMath::RandPointInCircle(DistractorSpawnRadius);
		const FVector SpawnLocation = BaseLocation + FVector(SpawnOffset2D.X, SpawnOffset2D.Y, 0.0f);
		AShellDistractorBall* DistractorBall = GetWorld()->SpawnActor<AShellDistractorBall>(
			SpawnClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParameters
		);

		if (!DistractorBall)
		{
			continue;
		}

		const FVector LaunchDirection = FMath::VRandCone(FVector::DownVector, ConeHalfAngleRadians).GetSafeNormal();
		const float LaunchSpeed = FMath::FRandRange(MinSpeed, MaxSpeed);
		const float BallScale = FMath::FRandRange(MinScale, MaxScale);

		DistractorBall->SetActorScale3D(FVector(BallScale));
		DistractorBall->Launch(LaunchDirection * LaunchSpeed);
		ActiveDistractorBalls.Add(DistractorBall);
	}
}

void AShellGameManager::ClearDistractorBalls()
{
	for (AShellDistractorBall* DistractorBall : ActiveDistractorBalls)
	{
		if (IsValid(DistractorBall))
		{
			DistractorBall->Destroy();
		}
	}

	ActiveDistractorBalls.Reset();
}
