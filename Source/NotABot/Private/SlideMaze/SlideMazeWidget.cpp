#include "SlideMaze/SlideMazeWidget.h"

#include "Components/TextBlock.h"
#include "SlideMaze/SlideMazeGameManager.h"

void USlideMazeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindToGameManager();
	ClearResult();
}

void USlideMazeWidget::NativeDestruct()
{
	UnbindFromGameManager();
	Super::NativeDestruct();
}

void USlideMazeWidget::SetGameManager(ASlideMazeGameManager* InManager)
{
	if (GameManager == InManager)
	{
		return;
	}

	UnbindFromGameManager();
	GameManager = InManager;
	BindToGameManager();

	if (GameManager)
	{
		UpdateTurnText(GameManager->CurrentTurns);
		ClearResult();
	}
}

void USlideMazeWidget::BindToGameManager()
{
	if (!GameManager)
	{
		return;
	}

	GameManager->OnTurnsChanged.AddUniqueDynamic(this, &USlideMazeWidget::UpdateTurnText);
	GameManager->OnGameSuccess.AddUniqueDynamic(this, &USlideMazeWidget::ShowSuccess);
	GameManager->OnGameFail.AddUniqueDynamic(this, &USlideMazeWidget::ShowFail);
}

void USlideMazeWidget::UnbindFromGameManager()
{
	if (!GameManager)
	{
		return;
	}

	GameManager->OnTurnsChanged.RemoveDynamic(this, &USlideMazeWidget::UpdateTurnText);
	GameManager->OnGameSuccess.RemoveDynamic(this, &USlideMazeWidget::ShowSuccess);
	GameManager->OnGameFail.RemoveDynamic(this, &USlideMazeWidget::ShowFail);
}

void USlideMazeWidget::UpdateTurnText(int32 Turns)
{
	if (TurnText)
	{
		TurnText->SetText(FText::Format(NSLOCTEXT("SlideMaze", "TurnsFormat", "Turns: {0}"), FText::AsNumber(Turns)));
	}
}

void USlideMazeWidget::ShowSuccess()
{
	if (ResultText)
	{
		ResultText->SetText(NSLOCTEXT("SlideMaze", "Success", "Success"));
		ResultText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void USlideMazeWidget::ShowFail()
{
	if (ResultText)
	{
		ResultText->SetText(NSLOCTEXT("SlideMaze", "Fail", "Fail"));
		ResultText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void USlideMazeWidget::ClearResult()
{
	if (ResultText)
	{
		ResultText->SetText(FText::GetEmpty());
		ResultText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
