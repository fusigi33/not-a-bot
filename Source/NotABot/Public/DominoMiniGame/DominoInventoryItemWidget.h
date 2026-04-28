#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DominoInventoryItemWidget.generated.h"

class ADominoBlockActor;
class UImage;
class USizeBox;

/**
 * 도미노 인벤토리 Horizontal Box에 들어가는 개별 도미노 아이콘 위젯입니다.
 *
 * 마우스 드래그 시작, 드래그 중 숨김, 취소 시 복구만 담당합니다.
 */
UCLASS()
class NOTABOT_API UDominoInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 드래그 중인 아이템을 숨기거나 다시 표시합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Inventory")
	void SetItemHiddenDuringDrag(bool bHidden);

	/** 현재 IconTexture를 화면에 표시되는 아이콘 이미지에 적용합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Inventory")
	void RefreshIconBrush();

	/** 이 아이템이 스폰할 도미노 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Inventory")
	TSubclassOf<ADominoBlockActor> DominoType;

	/** 아이콘 이미지입니다. Blueprint에서 같은 이름으로 바인딩할 수 있습니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UImage> DominoIconImage;

	/** 아이콘의 표시 크기를 고정할 SizeBox입니다. Blueprint에서 같은 이름으로 바인딩할 수 있습니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<USizeBox> DominoIconSizeBox;

	/** 드래그 비주얼로 사용할 위젯 클래스입니다. 없으면 자기 자신을 기본 비주얼로 사용하지 않고 간단한 Image를 생성합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Inventory")
	TSubclassOf<UUserWidget> DragVisualWidgetClass;

	/** 기본 드래그 비주얼 이미지에 사용할 텍스처입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Inventory")
	TObjectPtr<UTexture2D> IconTexture;

	/** 인벤토리에서 표시할 아이콘 높이입니다. 원본 109x304 비율을 유지해 너비를 계산합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Inventory|Layout", meta = (ClampMin = "1.0"))
	float IconDisplayHeight = 48.0f;

	/** 드래그 중 커서 옆에 표시할 보조 아이콘 높이입니다. 실제 배치 크기는 월드 프리뷰 도미노가 기준입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Inventory|Layout", meta = (ClampMin = "1.0"))
	float DragVisualDisplayHeight = 32.0f;

	/** 드래그 보조 아이콘 투명도입니다. 월드 프리뷰 도미노를 더 잘 보이게 하기 위해 낮게 둡니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Inventory|Layout", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DragVisualOpacity = 0.35f;

protected:
	virtual void NativePreConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** 드래그 비주얼을 생성합니다. */
	UWidget* CreateDragVisual();
};
