#include "DominoMiniGame/DominoInventoryItemWidget.h"

#include "DominoMiniGame/DominoBlockActor.h"
#include "DominoMiniGame/DominoDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogDominoInventoryItemWidget, Log, All);

namespace
{
	constexpr float DominoTextureWidth = 109.0f;
	constexpr float DominoTextureHeight = 304.0f;

	FVector2D GetDominoIconDisplaySize(float DisplayHeight)
	{
		const float SafeHeight = FMath::Max(1.0f, DisplayHeight);
		return FVector2D(SafeHeight * (DominoTextureWidth / DominoTextureHeight), SafeHeight);
	}
}

void UDominoInventoryItemWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshIconBrush();
}

void UDominoInventoryItemWidget::SetItemHiddenDuringDrag(bool bHidden)
{
	SetVisibility(bHidden ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void UDominoInventoryItemWidget::RefreshIconBrush()
{
	const FVector2D DisplaySize = GetDominoIconDisplaySize(IconDisplayHeight);

	if (DominoIconSizeBox)
	{
		DominoIconSizeBox->SetWidthOverride(DisplaySize.X);
		DominoIconSizeBox->SetHeightOverride(DisplaySize.Y);
	}

	if (DominoIconImage)
	{
		if (IconTexture)
		{
			DominoIconImage->SetBrushFromTexture(IconTexture, false);
		}

		DominoIconImage->SetDesiredSizeOverride(DisplaySize);
	}
}

FReply UDominoInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UDominoInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UE_LOG(LogDominoInventoryItemWidget, Warning, TEXT("Domino inventory drag detected. DominoType=%s"),
		DominoType ? *DominoType->GetName() : TEXT("None"));

	UDominoDragDropOperation* DragOperation = NewObject<UDominoDragDropOperation>(this);
	if (!DragOperation)
	{
		return;
	}

	SetItemHiddenDuringDrag(true);

	DragOperation->SourceItemWidget = this;
	DragOperation->DominoType = DominoType;
	DragOperation->DragVisual = nullptr;
	DragOperation->DefaultDragVisual = nullptr;
	DragOperation->Pivot = EDragPivot::MouseDown;

	OutOperation = DragOperation;
}

void UDominoInventoryItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	SetItemHiddenDuringDrag(false);
}

UWidget* UDominoInventoryItemWidget::CreateDragVisual()
{
	if (DragVisualWidgetClass)
	{
		UUserWidget* DragVisualWidget = CreateWidget<UUserWidget>(GetWorld(), DragVisualWidgetClass);
		if (DragVisualWidget)
		{
			DragVisualWidget->SetRenderOpacity(DragVisualOpacity);
		}

		return DragVisualWidget;
	}

	const FVector2D DisplaySize = GetDominoIconDisplaySize(DragVisualDisplayHeight);

	USizeBox* DragSizeBox = NewObject<USizeBox>(this);
	if (!DragSizeBox)
	{
		return nullptr;
	}

	DragSizeBox->SetWidthOverride(DisplaySize.X);
	DragSizeBox->SetHeightOverride(DisplaySize.Y);
	DragSizeBox->SetRenderOpacity(DragVisualOpacity);
	DragSizeBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	UImage* DragImage = NewObject<UImage>(DragSizeBox);
	if (!DragImage)
	{
		return DragSizeBox;
	}

	if (IconTexture)
	{
		DragImage->SetBrushFromTexture(IconTexture, false);
	}
	else if (DominoIconImage)
	{
		DragImage->SetBrush(DominoIconImage->GetBrush());
	}

	DragImage->SetDesiredSizeOverride(DisplaySize);
	DragSizeBox->SetContent(DragImage);

	return DragSizeBox;
}
