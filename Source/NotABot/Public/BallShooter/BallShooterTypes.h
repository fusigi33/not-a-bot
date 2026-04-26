#pragma once

#include "CoreMinimal.h"
#include "BallShooterTypes.generated.h"

class ABallShooterGoal;

/** 플레이어 조준 흐름의 현재 단계를 나타냅니다. */
UENUM(BlueprintType)
enum class EBallShooterAimState : uint8
{
	/** 발사 각도를 조절하는 상태입니다. */
	Aiming,
	/** 발사 속도를 충전하는 상태입니다. */
	Charging,
	/** 공이 이미 발사되어 조작이 끝난 상태입니다. */
	Launched,
	/** 라운드가 종료되어 입력을 받지 않는 상태입니다. */
	RoundEnded
};

/** 라운드 종료 결과를 나타냅니다. */
UENUM(BlueprintType)
enum class EBallShooterRoundResult : uint8
{
	/** 공이 목표에 도달해 성공한 상태입니다. */
	Success,
	/** 라운드 실패 상태입니다. */
	Failed,
	/** 공이 멈추거나 경계를 벗어나 도달 불가가 된 상태입니다. */
	Unreachable
};

/**
 * BallShooter 한 라운드의 시작 배치와 경계 규칙을 담는 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FBallShooterRoundData
{
	GENERATED_BODY()

	/** 공의 시작 위치와 회전입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallShooter|Round")
	FTransform BallStartTransform = FTransform::Identity;

	/** 목표 액터의 배치 위치와 회전입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallShooter|Round")
	FTransform GoalTransform = FTransform::Identity;

	/** 해당 라운드에서 허용할 최소 발사 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallShooter|Round")
	float MinLaunchSpeed = 900.0f;

	/** 해당 라운드에서 허용할 최대 발사 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallShooter|Round")
	float MaxLaunchSpeed = 2400.0f;

	/** 공이 벗어나면 실패 처리할 최소 보드 경계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallShooter|Round")
	FVector BoardMinBounds = FVector(-1500.0f, -1500.0f, -400.0f);

	/** 공이 벗어나면 실패 처리할 최대 보드 경계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BallShooter|Round")
	FVector BoardMaxBounds = FVector(1500.0f, 1500.0f, 900.0f);
};
