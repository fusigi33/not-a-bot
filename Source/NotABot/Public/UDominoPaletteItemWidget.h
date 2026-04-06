#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDominoPaletteItemWidget.generated.h"

UCLASS()
class NOTABOT_API UDominoPaletteItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Domino")
	TSubclassOf<class ADominoBlock> DominoClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Domino")
	float DefaultYaw = 0.0f;
	
	// 드래그할 때 마우스에 붙일 UI 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Domino")
	TSubclassOf<UUserWidget> DragVisualClass;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};