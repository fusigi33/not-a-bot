#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallShooterObstacleBase.generated.h"

class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * BallShooter 장애물의 공통 시각 요소와 디버그 기능을 제공하는 베이스 클래스입니다.
 */
UCLASS(Abstract)
class NOTABOT_API ABallShooterObstacleBase : public AActor
{
	GENERATED_BODY()

public:
	/** 기본 루트와 메시 컴포넌트를 초기화합니다. */
	ABallShooterObstacleBase();

protected:
	/** 시작 시 기준 위치와 회전을 저장하고 디버그 표시를 수행합니다. */
	virtual void BeginPlay() override;

	/** 장애물 공통 루트 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 장애물의 비주얼을 표현하는 메시 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UStaticMeshComponent> ObstacleMesh;

	/** BeginPlay 시 디버그 박스를 그릴지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Debug")
	bool bDrawDebug = false;

	/** 디버그 박스에 사용할 색상입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Debug")
	FColor DebugColor = FColor::Orange;

	/** 장애물의 초기 위치입니다. */
	FVector StartLocation = FVector::ZeroVector;
	/** 장애물의 초기 회전입니다. */
	FRotator StartRotation = FRotator::ZeroRotator;

	/** 파생 클래스 충돌 컴포넌트에 공통 충돌 응답을 적용합니다. */
	void ConfigureObstacleCollision(UPrimitiveComponent* CollisionComponent) const;

	/** 파생 클래스가 자신의 충돌 형태에 맞는 디버그 표시를 그립니다. */
	virtual void DrawObstacleDebug() const;
};
