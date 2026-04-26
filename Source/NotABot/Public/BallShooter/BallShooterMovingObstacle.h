#pragma once

#include "CoreMinimal.h"
#include "BallShooter/BallShooterObstacleBase.h"
#include "BallShooterMovingObstacle.generated.h"

class UBoxComponent;

/**
 * 시작 위치에서 로컬 축 한 방향으로 이동했다가 다시 돌아오는 장애물입니다.
 */
UCLASS()
class NOTABOT_API ABallShooterMovingObstacle : public ABallShooterObstacleBase
{
	GENERATED_BODY()

public:
	/** 이동 장애물 기본 설정을 초기화합니다. */
	ABallShooterMovingObstacle();

	/** 라운드 데이터 기반으로 이동 속도를 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Movement")
	void SetMoveSpeed(float InMoveSpeed);

	/** 시작 위치와 끝 지점 사이를 왕복하도록 장애물 위치를 매 프레임 갱신합니다. */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** 이동에 필요한 월드 축과 주기 값을 시작 시점에 캐시합니다. */
	virtual void BeginPlay() override;

	/** 이동 장애물의 단일 박스 충돌체입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	/** 왕복 이동에 사용할 로컬 기준 축입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Movement")
	FVector LocalMoveAxis = FVector::LeftVector;

	/** 시작 위치에서 왕복할 최대 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Movement", meta=(ClampMin="0.0"))
	float MoveDistance = 385.0f;

	/** 초당 이동 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Movement", meta=(ClampMin="0.0"))
	float MoveSpeed = 200.0f;

	/** 왕복 사이클의 초기 위상을 조정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Movement")
	float InitialPhase = 0.0f;

	/** 이동 장애물 충돌 기준으로 디버그 박스를 그립니다. */
	virtual void DrawObstacleDebug() const override;

private:
	/** 왕복 사이클 계산에 사용하는 누적 시간입니다. */
	float RunningTime = 0.0f;

	/** 이동에 사용하는 월드 기준 축 캐시입니다. */
	FVector CachedWorldAxis = FVector::ZeroVector;

	/** 왕복 이동 한 사이클의 총 길이 캐시입니다. */
	float CachedCycleLength = 0.0f;

	/** 초기 위상을 거리 값으로 변환해 저장한 캐시입니다. */
	float CachedPhaseDistance = 0.0f;
};
