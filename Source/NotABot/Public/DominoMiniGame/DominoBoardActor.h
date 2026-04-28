#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoBoardActor.generated.h"

/**
 * 도미노를 배치할 수 있는 보드 영역을 정의하는 액터입니다.
 *
 * 보드 중심, 보드 크기, 스냅 간격과 보드 평면 높이를 관리합니다.
 * 실제 렌더링 메시를 직접 소유하지 않아도 되며, 레벨의 보드 메시 위에 함께 배치해 사용합니다.
 */
UCLASS()
class NOTABOT_API ADominoBoardActor : public AActor
{
	GENERATED_BODY()

public:
	ADominoBoardActor();

	/** 월드 위치가 보드 사각 영역 안에 있는지 검사합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Domino Board")
	bool IsInsideBoard(const FVector& WorldLocation) const;

	/** 월드 위치를 보드 스냅 간격에 맞춰 보정합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Domino Board")
	FVector SnapLocationToBoard(const FVector& WorldLocation) const;

	/** 보드 월드 Bounds를 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Domino Board")
	FBox GetBoardBounds() const;

	/** 보드 중심 기준 로컬 X/Z 전체 크기입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Board", meta = (ClampMin = "1.0"))
	FVector2D BoardSize = FVector2D(900.0f, 500.0f);

	/** 도미노 배치 위치를 보정할 스냅 간격입니다. 0이면 스냅하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Board", meta = (ClampMin = "0.0"))
	float SnapSize = 25.0f;

	/** true이면 SnapSize 간격에 맞춰 배치하고, false이면 입력 위치를 그대로 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Board")
	bool bUsePlacementSnap = false;

	/** 도미노가 놓일 보드 평면의 월드 Z 위치입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Board")
	float BoardPlaneZ = 0.0f;

	/** Bounds 시각화를 위한 에디터 전용 높이입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Board")
	float BoundsHeight = 20.0f;
};
