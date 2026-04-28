#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DominoDragDropOperation.generated.h"

class UDominoInventoryItemWidget;
class ADominoBlockActor;

/**
 * UMG 드래그 중인 도미노 아이템의 정보를 담는 DragDropOperation입니다.
 */
UCLASS()
class NOTABOT_API UDominoDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 드래그를 시작한 인벤토리 아이템 위젯입니다. */
	UPROPERTY(BlueprintReadWrite, Category = "Domino Drag")
	TObjectPtr<UDominoInventoryItemWidget> SourceItemWidget;

	/** 드래그 중인 도미노 클래스입니다. */
	UPROPERTY(BlueprintReadWrite, Category = "Domino Drag")
	TSubclassOf<ADominoBlockActor> DominoType;

	/** Blueprint에서 참조하기 편한 별도 드래그 비주얼 포인터입니다. */
	UPROPERTY(BlueprintReadWrite, Category = "Domino Drag")
	TObjectPtr<UWidget> DragVisual;
};
