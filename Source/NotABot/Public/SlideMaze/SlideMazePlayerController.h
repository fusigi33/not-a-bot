#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SlideMaze/SlideMazeTypes.h"
#include "SlideMazePlayerController.generated.h"

class ASlideMazeGameManager;
class UInputAction;
class UInputMappingContext;
class USlideMazeWidget;
struct FInputActionValue;

UCLASS()
class NOTABOT_API ASlideMazePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Input")
	void AddSlideMazeInputMapping();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Input")
	void RemoveSlideMazeInputMapping();

	UFUNCTION(BlueprintCallable, Category="SlideMaze")
	void SetGameManager(ASlideMazeGameManager* InGameManager);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|UI")
	void CreateSlideMazeWidget();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|UI")
	void BindWidgetToGameManager();

	void HandleMove(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Input")
	void TryRequestMove(ESlideMazeDirection Direction);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Input")
	TObjectPtr<UInputMappingContext> SlideMazeMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|Input")
	int32 MappingPriority = 10;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="SlideMaze")
	TObjectPtr<ASlideMazeGameManager> GameManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|UI")
	TSubclassOf<USlideMazeWidget> SlideMazeWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|UI")
	TObjectPtr<USlideMazeWidget> SlideMazeWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze")
	bool bAutoFindGameManager = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlideMaze|UI")
	bool bCreateWidgetOnBeginPlay = true;
};
