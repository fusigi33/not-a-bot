#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShellGameWidget.generated.h"

class UImage;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShellBoardImageClicked, FVector2D, UV);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShellBoardImageHovered, FVector2D, UV);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShellBoardHoverEnded);

UCLASS()
class NOTABOT_API UShellGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SetBoardInputEnabled(bool bEnabled);

	UPROPERTY(BlueprintAssignable, Category="Shell Game")
	FOnShellBoardImageClicked OnBoardImageClicked;

	UPROPERTY(BlueprintAssignable, Category="Shell Game")
	FOnShellBoardImageHovered OnBoardImageHovered;

	UPROPERTY(BlueprintAssignable, Category="Shell Game")
	FOnShellBoardHoverEnded OnBoardHoverEnded;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_BoardView;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game")
	bool bBoardInputEnabled = false;
};
