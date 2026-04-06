#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DominoDragOperation.generated.h"

class UDominoPaletteItemWidget;

UCLASS()
class NOTABOT_API UDominoDragOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class ADominoBlock> DominoClass;

	UPROPERTY(BlueprintReadWrite)
	float InitialYaw = 0.0f;

	// 드래그를 시작한 원본 UI 위젯을 기억합니다.
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UDominoPaletteItemWidget> SourceWidget;
};