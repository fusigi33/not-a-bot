#include "ShellGame/ShellGameWidget.h"

#include "Components/Image.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

namespace
{
bool GetNormalizedBoardUV(UImage* BoardImage, const FVector2D& ScreenPosition, FVector2D& OutUV)
{
	if (!BoardImage)
	{
		return false;
	}

	const FGeometry ImageGeometry = BoardImage->GetCachedGeometry();
	const FVector2D LocalPosition = ImageGeometry.AbsoluteToLocal(ScreenPosition);
	const FVector2D LocalSize = ImageGeometry.GetLocalSize();

	if (LocalSize.X <= KINDA_SMALL_NUMBER || LocalSize.Y <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	if (LocalPosition.X < 0.0f || LocalPosition.Y < 0.0f || LocalPosition.X > LocalSize.X || LocalPosition.Y > LocalSize.Y)
	{
		return false;
	}

	OutUV = FVector2D(LocalPosition.X / LocalSize.X, LocalPosition.Y / LocalSize.Y);
	return true;
}
}

void UShellGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetBoardInputEnabled(false);
}

FReply UShellGameWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bBoardInputEnabled || !Image_BoardView || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	FVector2D BoardUV = FVector2D::ZeroVector;
	if (!GetNormalizedBoardUV(Image_BoardView, InMouseEvent.GetScreenSpacePosition(), BoardUV))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	OnBoardImageClicked.Broadcast(BoardUV);
	return FReply::Handled();
}

FReply UShellGameWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bBoardInputEnabled || !Image_BoardView)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	FVector2D BoardUV = FVector2D::ZeroVector;
	if (GetNormalizedBoardUV(Image_BoardView, InMouseEvent.GetScreenSpacePosition(), BoardUV))
	{
		OnBoardImageHovered.Broadcast(BoardUV);
	}
	else
	{
		OnBoardHoverEnded.Broadcast();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UShellGameWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	OnBoardHoverEnded.Broadcast();
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UShellGameWidget::SetBoardInputEnabled(bool bEnabled)
{
	bBoardInputEnabled = bEnabled;

	if (!bBoardInputEnabled)
	{
		OnBoardHoverEnded.Broadcast();
	}
}
