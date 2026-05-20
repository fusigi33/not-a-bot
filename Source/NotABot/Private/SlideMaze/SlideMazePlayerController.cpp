#include "SlideMaze/SlideMazePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "SlideMaze/SlideMazeGameManager.h"
#include "SlideMaze/SlideMazeWidget.h"

void ASlideMazePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoFindGameManager && !GameManager)
	{
		GameManager = Cast<ASlideMazeGameManager>(UGameplayStatics::GetActorOfClass(this, ASlideMazeGameManager::StaticClass()));
	}

	AddSlideMazeInputMapping();

	if (bCreateWidgetOnBeginPlay)
	{
		CreateSlideMazeWidget();
	}

	BindWidgetToGameManager();

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ASlideMazePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveSlideMazeInputMapping();
	Super::EndPlay(EndPlayReason);
}

void ASlideMazePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SlideMazePlayerController requires EnhancedInputComponent."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ASlideMazePlayerController::HandleMove);
	}
}

void ASlideMazePlayerController::AddSlideMazeInputMapping()
{
	if (!SlideMazeMappingContext)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->AddMappingContext(SlideMazeMappingContext, MappingPriority);
	}
}

void ASlideMazePlayerController::RemoveSlideMazeInputMapping()
{
	if (!SlideMazeMappingContext)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->RemoveMappingContext(SlideMazeMappingContext);
	}
}

void ASlideMazePlayerController::SetGameManager(ASlideMazeGameManager* InGameManager)
{
	GameManager = InGameManager;
	BindWidgetToGameManager();
}

void ASlideMazePlayerController::CreateSlideMazeWidget()
{
	if (SlideMazeWidget || !SlideMazeWidgetClass)
	{
		return;
	}

	SlideMazeWidget = CreateWidget<USlideMazeWidget>(this, SlideMazeWidgetClass);
	if (SlideMazeWidget)
	{
		SlideMazeWidget->AddToViewport();
	}
}

void ASlideMazePlayerController::BindWidgetToGameManager()
{
	if (SlideMazeWidget)
	{
		SlideMazeWidget->SetGameManager(GameManager);
	}
}

void ASlideMazePlayerController::HandleMove(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	if (MoveVector.IsNearlyZero())
	{
		return;
	}

	if (FMath::Abs(MoveVector.X) >= FMath::Abs(MoveVector.Y))
	{
		TryRequestMove(MoveVector.X > 0.0f ? ESlideMazeDirection::Right : ESlideMazeDirection::Left);
	}
	else
	{
		TryRequestMove(MoveVector.Y > 0.0f ? ESlideMazeDirection::Up : ESlideMazeDirection::Down);
	}
}

void ASlideMazePlayerController::TryRequestMove(ESlideMazeDirection Direction)
{
	if (!GameManager || !GameManager->CanAcceptInput())
	{
		return;
	}

	GameManager->RequestMove(Direction);
}
