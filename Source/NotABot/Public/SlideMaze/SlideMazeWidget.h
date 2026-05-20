#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlideMazeWidget.generated.h"

class ASlideMazeGameManager;
class UImage;
class UTextBlock;

UCLASS()
class NOTABOT_API USlideMazeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="SlideMaze|UI")
	void SetGameManager(ASlideMazeGameManager* InManager);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|UI")
	void BindToGameManager();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|UI")
	void UnbindFromGameManager();

	UFUNCTION()
	void UpdateTurnText(int32 Turns);

	UFUNCTION()
	void ShowSuccess();

	UFUNCTION()
	void ShowFail();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|UI")
	void ClearResult();

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="SlideMaze|UI")
	TObjectPtr<UImage> BoardImage;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="SlideMaze|UI")
	TObjectPtr<UTextBlock> TurnText;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="SlideMaze|UI")
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(BlueprintReadOnly, Category="SlideMaze|UI")
	TObjectPtr<ASlideMazeGameManager> GameManager;
};
