#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DominoPuzzleWidget.generated.h"

class UImage;
class ADominoPuzzleBoard;
class UDominoDragOperation;

UCLASS()
class NOTABOT_API UDominoPuzzleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_PuzzleView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	TObjectPtr<ADominoPuzzleBoard> PuzzleBoard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	float CurrentPlacementYaw = 0.0f;
	
	// 월드에 배치된 3D 액터와, 인벤토리의 숨겨진 2D 위젯을 짝지어 기억합니다.
	UPROPERTY()
	TMap<class ADominoBlock*, class UDominoPaletteItemWidget*> PlacedWidgetMap;

	UFUNCTION(BlueprintCallable)
	void SetPuzzleBoard(ADominoPuzzleBoard* InBoard);

	UFUNCTION(BlueprintCallable)
	void RotatePlacement(float DeltaYaw);

	UFUNCTION(BlueprintCallable)
	void StartPuzzle();

	UFUNCTION(BlueprintCallable)
	void ResetPuzzle();

protected:
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * 모니터 화면 상의 마우스 픽셀 좌표(절대 좌표)를 특정 UI 위젯 내부의 0.0 ~ 1.0 사이 비율 값(UV 좌표)으로 변환합니다.
	 * @param InGeometry 클릭이나 드롭이 발생한 대상 UI 위젯의 크기와 화면상 위치 정보입니다.
	 * @param ScreenSpacePos 사용자가 마우스를 클릭/드롭한 모니터 화면 전체 기준의 절대 픽셀 좌표입니다.
	 * @param OutUV 위젯의 좌측 상단을 (0, 0), 우측 하단을 (1, 1)로 보았을 때 마우스가 위치한 비율 값입니다.
	 * @return 계산된 UV 좌표가 위젯 영역 안에 정상적으로 존재하면 true, 밖을 벗어났거나 위젯 크기가 0이면 false를 반환합니다.
	 */
	bool GetUVFromGeometry(const FGeometry& InGeometry, const FVector2D& ScreenSpacePos, FVector2D& OutUV) const;
};