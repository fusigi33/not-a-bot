#include "DominoPuzzleWidget.h"
#include "DominoPuzzleBoard.h"
#include "DominoDragOperation.h"

#include "DominoBlock.h"
#include "UDominoPaletteItemWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UDominoPuzzleWidget::SetPuzzleBoard(ADominoPuzzleBoard* InBoard)
{
	PuzzleBoard = InBoard;
}

void UDominoPuzzleWidget::RotatePlacement(float DeltaYaw)
{
	CurrentPlacementYaw = FMath::Fmod(CurrentPlacementYaw + DeltaYaw, 360.0f);
	if (CurrentPlacementYaw < 0.0f)
	{
		CurrentPlacementYaw += 360.0f;
	}
}

void UDominoPuzzleWidget::StartPuzzle()
{
	if (PuzzleBoard)
	{
		PuzzleBoard->StartSimulation();
	}
}

void UDominoPuzzleWidget::ResetPuzzle()
{
	if (PuzzleBoard)
	{
		PuzzleBoard->ResetPuzzle();
	}
}

bool UDominoPuzzleWidget::GetUVFromGeometry(const FGeometry& InGeometry, const FVector2D& ScreenSpacePos,
                                            FVector2D& OutUV) const
{
	// 모니터 전체를 기준으로 한 마우스 좌표(ScreenSpacePos)를
	// 해당 UI 위젯의 좌측 상단을 (0, 0)으로 하는 위젯 내부 좌표(LocalPos)로 변환
	const FVector2D LocalPos = USlateBlueprintLibrary::AbsoluteToLocal(InGeometry, ScreenSpacePos);

	// 현재 화면에 렌더링된 위젯의 실제 가로/세로 픽셀 크기(예: 800x600)를 가져옴
	const FVector2D LocalSize = InGeometry.GetLocalSize();

	if (LocalSize.X <= KINDA_SMALL_NUMBER || LocalSize.Y <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// 마우스가 위젯 내에서 위치한 픽셀 좌표를 위젯의 전체 픽셀 크기로 나누어 비율(퍼센트)을 구함
	OutUV.X = LocalPos.X / LocalSize.X;
	OutUV.Y = LocalPos.Y / LocalSize.Y;

	return OutUV.X >= 0.0f && OutUV.X <= 1.0f && OutUV.Y >= 0.0f && OutUV.Y <= 1.0f;
}

bool UDominoPuzzleWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                           UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	UDominoDragOperation* DominoOp = Cast<UDominoDragOperation>(InOperation);
	if (!DominoOp || !PuzzleBoard) return false;

	bool bCanPlace = false;
	FVector2D UV;

	// 1. 현재 마우스 위치의 UV 좌표를 구합니다.
	if (GetUVFromGeometry(InGeometry, InDragDropEvent.GetScreenSpacePosition(), UV))
	{
		FTransform TestTransform;
		float FinalYaw = DominoOp->InitialYaw + CurrentPlacementYaw;

		// 2. UV 좌표가 실제 3D 월드에서 배치가 가능한지 가상으로 테스트합니다.
		if (PuzzleBoard->GetPlacementTransformFromUV(UV, FinalYaw, TestTransform))
		{
			bCanPlace = PuzzleBoard->CanPlaceAtTransform(TestTransform, DominoOp->DominoClass);
		}
	}

	// 3. 테스트 결과에 따라 마우스에 붙은 드래그 위젯의 색상을 바꿉니다.
	if (DominoOp->DefaultDragVisual)
	{
		// 놓을 수 있으면 흰색(원래 색상), 놓을 수 없으면 반투명한 빨간색
		FLinearColor TintColor = bCanPlace ? FLinearColor::White : FLinearColor(1.0f, 0.1f, 0.1f, 0.7f);

		// 위젯 자체의 투명도와 색상을 덮어씌웁니다.
		if (UUserWidget* UserDragVisual = Cast<UUserWidget>(DominoOp->DefaultDragVisual))
		{
			UserDragVisual->SetColorAndOpacity(TintColor);
		}
	}

	return true;
}

bool UDominoPuzzleWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                       UDragDropOperation* InOperation)
{
	if (!PuzzleBoard || !InOperation)
	{
		return false;
	}

	UDominoDragOperation* DominoOp = Cast<UDominoDragOperation>(InOperation);
	if (!DominoOp || !DominoOp->DominoClass)
	{
		return false;
	}

	FVector2D UV;
	const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();

	if (!GetUVFromGeometry(InGeometry, ScreenPos, UV))
	{
		return false;
	}

	const float FinalYaw = DominoOp->InitialYaw + CurrentPlacementYaw;

	ADominoBlock* PlacedBlock = PuzzleBoard->TryPlaceDominoFromUV(UV, DominoOp->DominoClass, FinalYaw);

	if (PlacedBlock)
	{
		// 배치가 성공했다면, 드래그했던 원본 UI 위젯을 소모(제거)합니다
		if (DominoOp->SourceWidget)
		{
			// Horizontal Box(부모)에서 이 위젯을 숨겨서 사용 처리합니다.
			DominoOp->SourceWidget->SetVisibility(ESlateVisibility::Collapsed);

			// 짝을 지어서 연락망에 기록합니다. (3D 블록이 키워드, 2D 위젯이 값)
			PlacedWidgetMap.Add(PlacedBlock, DominoOp->SourceWidget);
		}
		return true;
	}

	return false;
}

FReply UDominoPuzzleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭 삭제 처리
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && PuzzleBoard)
	{
		FVector2D UV;
		if (GetUVFromGeometry(InGeometry, InMouseEvent.GetScreenSpacePosition(), UV))
		{
			ADominoBlock* RemovedBlock = PuzzleBoard->TryRemoveDominoFromUV(UV);

			if (RemovedBlock)
			{
				// 연락망에 이 액터와 짝지어진 숨겨진 UI 위젯이 있는지 확인
				if (PlacedWidgetMap.Contains(RemovedBlock))
				{
					UDominoPaletteItemWidget* FoundWidget = PlacedWidgetMap[RemovedBlock];

					if (FoundWidget)
					{
						// 찾은 위젯을 다시 화면에 보이게 함
						FoundWidget->SetVisibility(ESlateVisibility::Visible);
					}

					// 다 쓴 연락망 기록을 지움
					PlacedWidgetMap.Remove(RemovedBlock);
				}

				// 3D 도미노 파괴
				RemovedBlock->Destroy();
				
				// 성공 처리
				return FReply::Handled();
			}
		}
	}

	// 우클릭이 아니었거나 삭제할 도미노가 없으면 부모 클래스의 기본 클릭 이벤트를 정상적으로 실행하도록 맨 밑에서 반환
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}