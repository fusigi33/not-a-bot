#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallShooterGoal.generated.h"

class ABallShooterPawn;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

/** 목표와 폰의 충돌 성공을 전달하는 델리게이트입니다. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallShooterGoalReachedSignature, ABallShooterGoal*, Goal, ABallShooterPawn*, Pawn);

/**
 * 폰이 도달해야 하는 목표 지점을 표현하는 액터입니다.
 * 오버랩 발생 시 폰에 성공 상태를 통지합니다.
 */
UCLASS()
class NOTABOT_API ABallShooterGoal : public AActor
{
	GENERATED_BODY()

public:
	/** 목표 트리거와 메시 컴포넌트를 초기화합니다. */
	ABallShooterGoal();

	/** 폰이 목표에 도달했을 때 브로드캐스트됩니다. */
	UPROPERTY(BlueprintAssignable, Category="BallShooter|Events")
	FOnBallShooterGoalReachedSignature OnGoalReached;

protected:
	/** 시작 시 오버랩 이벤트를 바인딩합니다. */
	virtual void BeginPlay() override;

	/** 목표 액터의 공통 기준점입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 폰 도착을 감지하는 박스 트리거입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UBoxComponent> GoalBoxTrigger;

	/** 목표의 비주얼을 표현하는 메시 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UStaticMeshComponent> GoalMesh;

	/**
	 * 목표 트리거에 공이 진입했을 때 성공 처리를 수행합니다.
	 * @param OverlappedComponent 오버랩된 컴포넌트입니다.
	 * @param OtherActor 오버랩을 발생시킨 액터입니다.
	 * @param OtherComp 오버랩을 발생시킨 컴포넌트입니다.
	 * @param OtherBodyIndex 오버랩 바디 인덱스입니다.
	 * @param bFromSweep 스윕 이동으로 발생한 오버랩 여부입니다.
	 * @param SweepResult 스윕 결과 정보입니다.
	 */
	UFUNCTION()
	void HandleGoalOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
