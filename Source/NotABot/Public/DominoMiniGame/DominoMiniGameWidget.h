#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DominoMiniGameWidget.generated.h"

class ADominoMiniGameManager;
class UButton;
class UHorizontalBox;
class UImage;
class UTextBlock;
class UDominoInventoryItemWidget;
class USceneCaptureComponent2D;

/**
 * 도미노 미니게임을 표시하고 입력을 월드 배치 요청으로 변환하는 메인 UMG 위젯입니다.
 *
 * Render Target Material이 적용된 BoardImage 위에서 드래그/드롭을 처리하고,
 * Orthographic SceneCapture 기준으로 UI 좌표를 보드 월드 좌표로 변환합니다.
 */
UCLASS()
class NOTABOT_API UDominoMiniGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 매니저를 연결하고 인벤토리 아이콘을 생성합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Widget")
	void InitializeDominoWidget(ADominoMiniGameManager* InManager);

	/** 라운드 데이터의 도미노 개수만큼 인벤토리 아이콘을 생성합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Widget")
	void CreateDominoInventoryIcons();

	/** BoardImage 위의 스크린 좌표를 월드 보드 위치로 변환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Widget")
	bool ConvertBoardImageScreenPositionToWorld(const FGeometry& BoardGeometry, const FVector2D& ScreenPosition, FVector& OutWorldLocation) const;

	/** 시작 버튼 클릭 처리입니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Widget")
	void HandleStartButtonClicked();

	/** 리셋 버튼 클릭 처리입니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Widget")
	void HandleResetButtonClicked();

	/** Render Target Material을 표시하는 보드 이미지입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UImage> BoardImage;

	/** 도미노 인벤토리 아이콘이 들어갈 Horizontal Box입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UHorizontalBox> DominoInventoryBox;

	/** 시뮬레이션 시작 버튼입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> StartButton;

	/** 라운드 초기화 버튼입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> ResetButton;

	/** 안내 문구 텍스트입니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> HeaderText;

	/** 연결할 도미노 미니게임 매니저입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Widget")
	TObjectPtr<ADominoMiniGameManager> Manager;

	/** 보드를 촬영하는 Orthographic SceneCapture 컴포넌트입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Widget|Capture")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	/** BoardImage에 적용할 Render Target Material입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Widget|Capture")
	TObjectPtr<UMaterialInterface> RenderTargetMaterial;

	/** 생성할 인벤토리 아이템 위젯 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Widget|Inventory")
	TSubclassOf<UDominoInventoryItemWidget> InventoryItemWidgetClass;

	/** 기본 아이템 위젯이 없을 때 드래그 비주얼에 사용할 텍스처입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Widget|Inventory")
	TObjectPtr<UTexture2D> DominoIconTexture;

	/** 보드 평면의 월드 Z입니다. BoardActor가 있으면 BoardActor 값이 우선됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Widget|Capture")
	float BoardPlaneZ = 0.0f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
