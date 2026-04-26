#pragma once

#include "CoreMinimal.h"
#include "BallShooter/BallShooterObstacleBase.h"
#include "BallShooterRotatingObstacle.generated.h"

class UBoxComponent;

/**
 * 지정한 로컬 축을 기준으로 지속 회전하는 장애물입니다.
 */
UCLASS()
class NOTABOT_API ABallShooterRotatingObstacle : public ABallShooterObstacleBase
{
	GENERATED_BODY()

public:
	/** 회전 장애물 기본 설정을 초기화합니다. */
	ABallShooterRotatingObstacle();

	/** 라운드 데이터 기반으로 회전 방향과 속도를 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Rotation")
	void SetRotationSpeed(float InDegreesPerSecond, bool bClockwise);

	/** 블레이드 충돌 배치를 에디터/런타임 구성값에 맞게 갱신합니다. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** 로컬 축 기준 회전을 매 프레임 적용합니다. */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** 회전 장애물의 블레이드 충돌들을 그립니다. */
	virtual void DrawObstacleDebug() const override;

	/** 블레이드 충돌 박스들의 활성 상태를 현재 설정에 맞게 갱신합니다. */
	void UpdateBladeCollisionState();
	
	/** 현재까지 누적된 로컬 회전 각도입니다. */
	float AccumulatedAngle = 0.0f;

	/** 풍차 블레이드 충돌체 배열입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TArray<TObjectPtr<UBoxComponent>> BladeCollisions;

	/** 블레이드 개수입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rotation", meta=(ClampMin="1", ClampMax="8"))
	int32 BladeCount = 4;

	/** 회전에 사용할 로컬 기준 축입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rotation")
	FVector LocalRotationAxis = FVector::UpVector;

	/** 초당 회전 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rotation")
	float DegreesPerSecond = 120.0f;
};
