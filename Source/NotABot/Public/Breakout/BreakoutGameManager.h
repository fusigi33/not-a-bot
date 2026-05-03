#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "BreakoutGameManager.generated.h"

class ABreakoutBall;
class ABreakoutBrick;
class ABreakoutCaptureActor;
class UBreakoutHUDWidget;
class UMaterialInterface;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBreakoutGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBreakoutMiniGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBreakoutMiniGameCleared);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBreakoutBrickDestroyed, AActor*, DestroyedActor);

UCLASS()
class NOTABOT_API ABreakoutGameManager : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutGameManager();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Breakout")
	void StartMiniGame();

	UFUNCTION(BlueprintCallable, Category="Breakout")
	void StopMiniGame();

	UFUNCTION(BlueprintCallable, Category="Breakout")
	void RestartBreakoutRound();

	UFUNCTION(BlueprintCallable, Category="Breakout")
	void HandleBallLost(ABreakoutBall* LostBall);

	UFUNCTION(BlueprintCallable, Category="Breakout")
	void HandleBrickReachedKillZone(ABreakoutBrick* Brick);

	UFUNCTION(BlueprintCallable, Category="Breakout")
	void HandleBrickTouchedPaddle(ABreakoutBrick* Brick);

	UFUNCTION(BlueprintCallable, Category="Breakout|Bricks")
	void SpawnBrickGrid(int32 RowCount, int32 ColumnCount, FVector StartLocation);

	UFUNCTION(BlueprintCallable, Category="Breakout|Bricks")
	void ClearSpawnedBricks();

	UFUNCTION(BlueprintCallable, Category="Breakout|Bricks")
	void StartBrickDropTimer();

	UFUNCTION(BlueprintCallable, Category="Breakout|Bricks")
	void StopBrickDropTimer();

	UFUNCTION(BlueprintCallable, Category="Breakout|UI")
	void FindOrCreateHUDWidget();

	UFUNCTION(BlueprintCallable, Category="Breakout|UI")
	void SetWidgetVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category="Breakout")
	bool IsMiniGameActive() const { return bMiniGameActive; }

	UPROPERTY(BlueprintAssignable, Category="Breakout")
	FOnBreakoutMiniGameStarted OnMiniGameStarted;

	UPROPERTY(BlueprintAssignable, Category="Breakout")
	FOnBreakoutGameOver OnGameOver;

	UPROPERTY(BlueprintAssignable, Category="Breakout")
	FOnBreakoutMiniGameCleared OnMiniGameCleared;

	UPROPERTY(BlueprintAssignable, Category="Breakout|Bricks")
	FOnBreakoutBrickDestroyed OnBrickDestroyed;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Breakout|Gameplay")
	TObjectPtr<ABreakoutBall> BallToControl;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Bricks")
	TSubclassOf<ABreakoutBrick> BrickClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breakout|Bricks")
	bool bEnableTimedBrickDrop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breakout|Bricks", meta=(ClampMin="0.1"))
	float BrickDropInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breakout|Bricks", meta=(ClampMin="0.01"))
	float BrickDropRetryInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Bricks", meta=(ClampMin="0.0"))
	float BrickDropDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Bricks")
	bool bSkipBrickDropWhenBallWouldBeTrapped = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Bricks", meta=(ClampMin="0.0"))
	float BrickBallSafetyPadding = 4.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Breakout|UI")
	TObjectPtr<ABreakoutCaptureActor> CaptureActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|UI")
	TSubclassOf<UBreakoutHUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|UI")
	TObjectPtr<UMaterialInterface> CaptureUIMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|UI")
	int32 WidgetZOrder = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Flow")
	bool bAutoStartOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|UI")
	bool bCreateWidgetIfMissing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|UI")
	bool bShowMouseCursorDuringMiniGame = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Breakout|UI")
	TObjectPtr<UBreakoutHUDWidget> ActiveHUDWidget;

	UPROPERTY(BlueprintReadOnly, Category="Breakout|Flow")
	bool bMiniGameActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Breakout|Flow")
	bool bIsGameOver = false;

	UPROPERTY(BlueprintReadOnly, Category="Breakout|Flow")
	bool bIsGameCleared = false;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ABreakoutBrick>> SpawnedBricks;

	FTimerHandle BrickDropTimerHandle;
	bool bSuppressBrickDestroyedNotifications = false;

	UTextureRenderTarget2D* ResolveRenderTarget() const;
	void ConfigureWidgetMaterial() const;
	void ScheduleNextBrickDrop(float DelaySeconds);
	void MoveBricksDownOneRow();
	float ResolveBrickDropDistance() const;
	bool WouldBrickDropTrapBall(float DropDistance) const;
	void RemoveInvalidBrickReferences();
	void TriggerGameOver();
	void HandleMiniGameCleared();

	UFUNCTION()
	void HandleBrickDestroyed(AActor* DestroyedActor);
};
