#include "PathMemory/PathTraceWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"

void UPathTraceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BoardViewMaterial && Image_BoardView)
	{
		BoardMID = UMaterialInstanceDynamic::Create(BoardViewMaterial, this);
		Image_BoardView->SetBrushFromMaterial(BoardMID);

		if (BoardMID && PendingBoardRT)
		{
			BoardMID->SetTextureParameterValue(TEXT("BoardRT"), PendingBoardRT);
		}
	}
}

void UPathTraceWidget::SetBoardRenderTarget(UTextureRenderTarget2D* InRT)
{
	PendingBoardRT = InRT;

	if (BoardMID && InRT)
	{
		BoardMID->SetTextureParameterValue(TEXT("BoardRT"), InRT);
	}
}

void UPathTraceWidget::ShowResult(bool bSuccess, float AccuracyPercent)
{
	// if (!Txt_Result)
	// {
	// 	return;
	// }
	//
	// const FString ResultText = FString::Printf(
	// 	TEXT("%s\nAccuracy: %.1f%%"),
	// 	bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"),
	// 	AccuracyPercent
	// );
	//
	// Txt_Result->SetText(FText::FromString(ResultText));
}