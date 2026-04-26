#include "Breakout/BreakoutGameManager.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Breakout/BreakoutBall.h"
#include "Breakout/BreakoutCaptureActor.h"
#include "Breakout/BreakoutHUDWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

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

	FindOrCreateHUDWidget();
	SetWidgetVisible(true);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->bShowMouseCursor = bShowMouseCursorDuringMiniGame;
	}

	if (BallToControl)
	{
		BallToControl->SetGameManager(this);
		BallToControl->ResetBall();
	}

	OnMiniGameStarted.Broadcast();
}

void ABreakoutGameManager::StopMiniGame()
{
	bMiniGameActive = false;

	if (BallToControl)
	{
		BallToControl->StopBall();
	}

	SetWidgetVisible(false);
}

void ABreakoutGameManager::RestartBreakoutRound()
{
	bIsGameOver = false;
	bMiniGameActive = true;

	if (BallToControl)
	{
		BallToControl->SetGameManager(this);
		BallToControl->ResetBall();
	}
}

void ABreakoutGameManager::HandleBallLost(ABreakoutBall* LostBall)
{
	if (!bMiniGameActive || bIsGameOver)
	{
		return;
	}

	bIsGameOver = true;
	bMiniGameActive = false;

	if (LostBall)
	{
		LostBall->StopBall();
	}

	OnGameOver.Broadcast();
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
