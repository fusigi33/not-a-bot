#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellGame/ShellGameTypes.h"
#include "ShellGameManager.generated.h"

class AShellBoard;
class AShellCaptureRig;
class AShellCup;
class AShellDistractorBall;
class AShellObstacleMover;
class APlayerController;
class UShellGameWidget;
class UShellSelectionResolver;

UCLASS()
class NOTABOT_API AShellGameManager : public AActor
{
	GENERATED_BODY()

public:
	AShellGameManager();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void StartShellGame();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void RequestResetRound();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SubmitCupSelection(AShellCup* SelectedCup);

	UFUNCTION(BlueprintCallable, Category="Shell Game|UI")
	void FindAndBindExistingWidget();

	UFUNCTION(BlueprintPure, Category="Shell Game")
	EShellGameState GetCurrentState() const { return CurrentState; }

protected:
	void TransitionToState(EShellGameState NewState);
	void UpdateHoveredCup(AShellCup* NewHoveredCup);
	void ExecuteStartShellGame();
	void ExecuteResetRound();
	void PrepareRoundForRestart();
	void BeginRoundAfterDelay();

	UFUNCTION()
	void HandleBoardImageClicked(FVector2D UV);

	UFUNCTION()
	void HandleBoardImageHovered(FVector2D UV);

	UFUNCTION()
	void HandleBoardHoverEnded();

	UFUNCTION()
	void HandleBoardIntroFinished();

	UFUNCTION()
	void HandleBoardShuffleFinished();

	UFUNCTION()
	void HandleBoardRevealFinished(const FShellSelectionResult& Result);

	void SetObstacleActivation(bool bActive);
	void ConfigurePlayerInput() const;
	bool CanAcceptSelection() const;
	void StartRoundInternal();
	void SpawnDistractorBalls();
	void ClearDistractorBalls();

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellBoard> Board;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellCaptureRig> CaptureRig;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Shell Game")
	TArray<TObjectPtr<AShellObstacleMover>> Obstacles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|UI")
	TSubclassOf<UShellGameWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Flow")
	bool bAutoStartOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Flow")
	bool bAutoRestartAfterReveal = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Flow")
	float AutoStartDelay = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Flow")
	float AutoRestartDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Selection")
	float SelectionTraceDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Selection")
	TEnumAsByte<ECollisionChannel> SelectionTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game|Distractor")
	bool bSpawnDistractorBallsOnRoundStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	TSubclassOf<AShellDistractorBall> DistractorBallClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0", UIMin="0"))
	int32 DistractorBallCount = 36;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	FVector DistractorSpawnOffset = FVector(0.0f, 0.0f, 460.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0.0", UIMin="0.0"))
	float DistractorSpawnRadius = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0.0", UIMin="0.0"))
	float DistractorInitialSpeedMin = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0.0", UIMin="0.0"))
	float DistractorInitialSpeedMax = 760.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0.0", ClampMax="89.0", UIMin="0.0", UIMax="89.0"))
	float DistractorConeHalfAngleDegrees = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0.0", UIMin="0.0"))
	float DistractorBallScaleMin = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor", meta=(ClampMin="0.0", UIMin="0.0"))
	float DistractorBallScaleMax = 1.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	EShellGameState CurrentState = EShellGameState::Idle;

	UPROPERTY(Transient)
	TObjectPtr<UShellGameWidget> ShellGameWidget;

	UPROPERTY(Transient)
	TObjectPtr<UShellSelectionResolver> SelectionResolver;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(Transient)
	TObjectPtr<AShellCup> HoveredCup;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AShellDistractorBall>> ActiveDistractorBalls;

private:
	FTimerHandle AutoStartTimerHandle;
	FTimerHandle AutoRestartTimerHandle;
	FTimerHandle PendingStartTimerHandle;
	FTimerHandle PendingResetTimerHandle;
};
