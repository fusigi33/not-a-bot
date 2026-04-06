#include "UDominoPaletteItemWidget.h"
#include "DominoDragOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

FReply UDominoPaletteItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void UDominoPaletteItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UDominoDragOperation* DragOp = NewObject<UDominoDragOperation>();
	DragOp->Pivot = EDragPivot::TopCenter;
	DragOp->DominoClass = DominoClass;
	DragOp->InitialYaw = DefaultYaw;
	DragOp->SourceWidget = this;
	
	// 에디터에서 할당한 Visual 위젯을 생성해서 마우스에 부착
	if (DragVisualClass)
	{
		UUserWidget* DragVisual = CreateWidget<UUserWidget>(GetWorld(), DragVisualClass);
		DragOp->DefaultDragVisual = DragVisual;
	}
	
	OutOperation = DragOp;
}