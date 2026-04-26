#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutGameManager.generated.h"

class ABreakoutBall;
class ABreakoutCaptureActor;
class UBreakoutHUDWidget;
class UMaterialInterface;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBreakoutGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBreakoutMiniGameStarted);

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

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Breakout|Gameplay")
	TObjectPtr<ABreakoutBall> BallToControl;

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

private:
	UTextureRenderTarget2D* ResolveRenderTarget() const;
	void ConfigureWidgetMaterial() const;
};
