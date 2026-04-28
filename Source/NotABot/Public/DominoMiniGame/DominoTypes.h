#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DominoTypes.generated.h"

class AActor;
class ADominoBlockActor;

/**
 * 시작 도미노가 처음 쓰러질 방향입니다.
 */
UENUM(BlueprintType)
enum class EDominoFallDirection : uint8
{
	Backward UMETA(DisplayName = "Backward"),
	Forward UMETA(DisplayName = "Forward")
};

/**
 * 도미노 미니게임의 현재 라운드 상태입니다.
 */
UENUM(BlueprintType)
enum class EDominoMiniGameState : uint8
{
	None UMETA(DisplayName = "None"),
	Placement UMETA(DisplayName = "Placement"),
	Simulating UMETA(DisplayName = "Simulating"),
	Success UMETA(DisplayName = "Success"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * 도미노 라운드에서 함께 배치할 상호작용 오브젝트 데이터입니다.
 */
USTRUCT(BlueprintType)
struct NOTABOT_API FDominoRoundInteractiveObjectData
{
	GENERATED_BODY()

	/** 스폰할 상호작용 오브젝트 액터 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	TSubclassOf<AActor> ObjectClass;

	/** 오브젝트를 배치할 월드 위치와 회전/스케일입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	FTransform Transform = FTransform::Identity;
};

/**
 * 도미노 라운드를 시작할 때 사용하는 데이터입니다.
 *
 * 시작/목표 도미노의 위치, 플레이어가 사용할 수 있는 도미노 개수,
 * 상호작용 오브젝트, 시뮬레이션 제한 시간과 기본 회전을 Blueprint에서 조정할 수 있습니다.
 */
USTRUCT(BlueprintType)
struct NOTABOT_API FDominoRoundData : public FTableRowBase
{
	GENERATED_BODY()

	/** 플레이어에게 제공할 배치용 도미노 개수입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	int32 AvailableDominoCount = 8;

	/** 시작 후 목표 도미노가 넘어져야 하는 제한 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round", meta = (ClampMin = "0.1"))
	float SimulationTimeLimit = 8.0f;

	/** 시작 도미노가 처음 쓰러질 방향입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	EDominoFallDirection InitialFallDirection = EDominoFallDirection::Forward;

	/** 스폰할 도미노 액터 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	TSubclassOf<ADominoBlockActor> DominoClass;

	/** 시작 도미노를 자동 스폰할 때 사용할 월드 위치입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	FVector StartDominoLocation = FVector::ZeroVector;

	/** 목표 도미노를 자동 스폰할 때 사용할 월드 위치입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	FVector GoalDominoLocation = FVector(500.0f, 0.0f, 0.0f);

	/** 도미노 배치 시 기본으로 적용할 회전입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	FRotator DefaultDominoRotation = FRotator::ZeroRotator;

	/** 도미노와 함께 라운드에 배치할 상호작용 오브젝트 목록입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Round")
	TArray<FDominoRoundInteractiveObjectData> InteractiveObjects;
};
