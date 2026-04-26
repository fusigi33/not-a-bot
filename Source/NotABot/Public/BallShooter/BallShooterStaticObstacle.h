#pragma once

#include "CoreMinimal.h"
#include "BallShooter/BallShooterObstacleBase.h"
#include "BallShooterStaticObstacle.generated.h"

class UBoxComponent;

/**
 * 고정 배치만 담당하는 정적 장애물입니다.
 */
UCLASS()
class NOTABOT_API ABallShooterStaticObstacle : public ABallShooterObstacleBase
{
	GENERATED_BODY()

public:
	/** 정적 장애물 기본 설정을 초기화합니다. */
	ABallShooterStaticObstacle();

protected:
	/** 정적 장애물의 단일 박스 충돌체입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	/** 정적 장애물 충돌 기준으로 디버그 박스를 그립니다. */
	virtual void DrawObstacleDebug() const override;
};
