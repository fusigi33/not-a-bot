#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoMiniGame/DominoSimulationObjectInterface.h"
#include "DominoSeesawBatteringRamActor.generated.h"

class USceneComponent;
class UBoxComponent;
class UStaticMeshComponent;

/**
 * 공이 충돌한 바구니의 방향을 나타냅니다.
 */
UENUM(BlueprintType)
enum class EDominoSeesawBasketSide : uint8
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FDominoSeesawBallLaunchedSignature,
	EDominoSeesawBasketSide, ImpactSide,
	EDominoSeesawBasketSide, LaunchSide,
	UPrimitiveComponent*, ImpactingBallComponent,
	UPrimitiveComponent*, LaunchedBallComponent);

/**
 * 중앙 받침점을 기준으로 시소처럼 작동하는 공성추/투석 오브젝트입니다.
 *
 * 한쪽 바구니에 공이 떨어지면 반대쪽 바구니의 공을 LaunchAngleDegrees 각도로 발사합니다.
 */
UCLASS()
class NOTABOT_API ADominoSeesawBatteringRamActor : public AActor, public IDominoSimulationObjectInterface
{
	GENERATED_BODY()

public:
	/** 기본 컴포넌트 계층과 충돌 설정을 구성합니다. */
	ADominoSeesawBatteringRamActor();

	/**
	 * 시소 막대의 현재 회전을 목표 기울기로 보간합니다.
	 *
	 * @param DeltaSeconds 이전 프레임 이후 경과한 시간입니다.
	 */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * 도미노 시뮬레이션 작동 여부를 설정합니다.
	 *
	 * @param bEnabled true이면 바구니 충돌 박스를 활성화하고, false이면 대기 상태로 복귀합니다.
	 */
	virtual void SetDominoSimulationEnabled_Implementation(bool bEnabled) override;

	/** 액터를 초기 배치와 기본 시소 상태로 되돌립니다. */
	virtual void ResetDominoSimulationObject_Implementation() override;

	/** 전체 컴포넌트 계층의 루트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 시소 막대를 지탱하는 중앙 받침대 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UStaticMeshComponent> FulcrumMesh;

	/** 시소 막대와 양쪽 바구니 메시만 함께 회전시키는 시각 연출 루트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<USceneComponent> BeamRoot;

	/** 중앙 받침점 위에서 기울어지는 시소 막대 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UStaticMeshComponent> BeamMesh;

	/** 왼쪽 바구니를 표현하는 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UStaticMeshComponent> LeftBasketMesh;

	/** 오른쪽 바구니를 표현하는 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UStaticMeshComponent> RightBasketMesh;

	/** 왼쪽 바구니 바닥 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> LeftBasketFloorCollision;

	/** 왼쪽 바구니 앞쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> LeftBasketFrontCollision;

	/** 왼쪽 바구니 뒤쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> LeftBasketBackCollision;

	/** 왼쪽 바구니 안쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> LeftBasketInnerCollision;

	/** 왼쪽 바구니 바깥쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> LeftBasketOuterCollision;

	/** 오른쪽 바구니 바닥 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> RightBasketFloorCollision;

	/** 오른쪽 바구니 앞쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> RightBasketFrontCollision;

	/** 오른쪽 바구니 뒤쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> RightBasketBackCollision;

	/** 오른쪽 바구니 안쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> RightBasketInnerCollision;

	/** 오른쪽 바구니 바깥쪽 벽 충돌입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> RightBasketOuterCollision;

	/** 왼쪽 바구니에서 물리 오브젝트를 막는 충돌 박스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> LeftBasketBlocker;

	/** 오른쪽 바구니에서 물리 오브젝트를 막는 충돌 박스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Components")
	TObjectPtr<UBoxComponent> RightBasketBlocker;

	/** 중앙 받침점에서 각 바구니 중심까지의 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Layout", meta = (ClampMin = "1.0"))
	float BasketOffset = 180.0f;

	/** 반대편 공을 발사할 포물선 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float LaunchAngleDegrees = 45.0f;

	/** 반대편 공의 초기 발사 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0"))
	float LaunchSpeed = 1200.0f;

	/** 트리거한 공도 바구니 안에서 살짝 눌러주는 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0"))
	float ImpactSinkSpeed = 80.0f;

	/** 이 속도보다 느린 바구니 충돌은 정지 접촉으로 보고 발사를 무시합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0"))
	float MinimumBasketImpactSpeed = 30.0f;

	/** 시뮬레이션 시작 직후 기존 접촉에서 발생하는 Overlap 이벤트를 무시하는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0"))
	float BasketBlockerActivationGraceTime = 0.1f;

	/** Block 충돌 박스 표면에 걸친 공까지 찾기 위해 발사 대상 검색 영역에 더하는 여유값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0"))
	float BasketSearchPadding = 60.0f;

	/** 한 번 작동한 뒤 다시 작동하기까지의 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Launch", meta = (ClampMin = "0.0"))
	float RearmDelay = 0.35f;

	/** 작동 시 시소 막대가 기울어지는 최대 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Motion", meta = (ClampMin = "0.0"))
	float MaxTiltDegrees = 28.0f;

	/** 시소 연출이 목표 기울기에 도달하는 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Seesaw Ram|Motion", meta = (ClampMin = "0.1"))
	float TiltInterpSpeed = 12.0f;

	/**
	 * 충돌한 바구니의 반대편 바구니에 있는 공을 발사합니다.
	 *
	 * @param ImpactSide 공이 들어온 바구니 방향입니다.
	 * @param ImpactingBallComponent 시소를 작동시킨 공 컴포넌트입니다.
	 * @return 반대편 공을 찾아 발사했으면 true, 발사하지 못했으면 false입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Domino Seesaw Ram")
	bool TryLaunchOppositeBasket(EDominoSeesawBasketSide ImpactSide, UPrimitiveComponent* ImpactingBallComponent);

	/** 반대편 바구니의 공이 실제로 발사된 직후 블루프린트에 알립니다. */
	UPROPERTY(BlueprintAssignable, Category = "Domino Seesaw Ram|Events")
	FDominoSeesawBallLaunchedSignature OnBallLaunched;

protected:
	/** 런타임 초기 상태를 저장하고 충돌 이벤트를 바인딩합니다. */
	virtual void BeginPlay() override;

	/**
	 * 에디터와 런타임 생성 시 바구니 메시 배치를 컴포넌트에 반영합니다.
	 *
	 * @param Transform 생성 시점의 액터 트랜스폼입니다.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * 왼쪽 바구니 트리거의 Overlap 이벤트를 처리합니다.
	 *
	 * @param OverlappedComponent 진입을 감지한 바구니 트리거입니다.
	 * @param OtherActor 바구니에 들어온 액터입니다.
	 * @param OtherComp 바구니에 들어온 컴포넌트입니다.
	 * @param OtherBodyIndex 겹친 바디 인덱스입니다.
	 * @param bFromSweep 스윕 이동 중 발생한 진입인지 여부입니다.
	 * @param SweepResult 진입 결과 정보입니다.
	 */
	UFUNCTION()
	void HandleLeftBasketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 오른쪽 바구니 트리거의 Overlap 이벤트를 처리합니다.
	 *
	 * @param OverlappedComponent 진입을 감지한 바구니 트리거입니다.
	 * @param OtherActor 바구니에 들어온 액터입니다.
	 * @param OtherComp 바구니에 들어온 컴포넌트입니다.
	 * @param OtherBodyIndex 겹친 바디 인덱스입니다.
	 * @param bFromSweep 스윕 이동 중 발생한 진입인지 여부입니다.
	 * @param SweepResult 진입 결과 정보입니다.
	 */
	UFUNCTION()
	void HandleRightBasketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 바구니에 들어온 물리 컴포넌트를 검증하고 발사를 시도합니다.
	 *
	 * @param ImpactSide 진입이 발생한 바구니 방향입니다.
	 * @param OtherComp 바구니에 들어온 컴포넌트입니다.
	 */
	void HandleBasketOverlap(EDominoSeesawBasketSide ImpactSide, UPrimitiveComponent* OtherComp);

	/**
	 * 지정한 바구니 충돌 박스 영역 안에서 발사 가능한 물리 컴포넌트를 찾습니다.
	 *
	 * @param BasketSide 검색할 바구니 방향입니다.
	 * @param ComponentToIgnore 충돌을 발생시킨 컴포넌트처럼 검색에서 제외할 컴포넌트입니다.
	 * @return 발사 가능한 컴포넌트입니다. 없으면 nullptr입니다.
	 */
	UPrimitiveComponent* FindLaunchableBallInBasket(EDominoSeesawBasketSide BasketSide, UPrimitiveComponent* ComponentToIgnore) const;

	/** 지정한 바구니에 현재 대기 중인 공 컴포넌트를 반환합니다. */
	UPrimitiveComponent* GetBasketBallComponent(EDominoSeesawBasketSide BasketSide) const;

	/** 지정한 바구니의 대기 공 컴포넌트를 갱신합니다. */
	void SetBasketBallComponent(EDominoSeesawBasketSide BasketSide, UPrimitiveComponent* BallComponent);

	/** 시뮬레이션 시작 시 이미 바구니 안에 있는 공을 대기 공으로 등록하고 고정합니다. */
	void RegisterExistingBasketBalls();

	/** 지정한 바구니 안에 이미 들어와 있는 공을 대기 상태로 고정합니다. */
	void RegisterExistingBasketBall(EDominoSeesawBasketSide BasketSide);

	/**
	 * 지정한 방향으로 공을 날릴 초기 속도 벡터를 계산합니다.
	 *
	 * @param LaunchSide 공을 발사할 바구니 방향입니다.
	 * @return LaunchAngleDegrees와 LaunchSpeed를 반영한 월드 공간 속도입니다.
	 */
	FVector BuildLaunchVelocity(EDominoSeesawBasketSide LaunchSide) const;

	/** 현재 배치 설정값을 바구니 메시 위치에 반영합니다. */
	void ApplyLayout();

	/**
	 * 발사 연출용 바구니 메시 하위 충돌들의 활성화 상태를 변경합니다.
	 *
	 * @param BasketSide 변경할 바구니 방향입니다.
	 * @param bEnabled true이면 BlockAll, false이면 NoCollision으로 설정합니다.
	 */
	void SetBasketMeshCollisionBoxesEnabled(EDominoSeesawBasketSide BasketSide, bool bEnabled) const;

	/**
	 * 양쪽 바구니 충돌 박스의 충돌 활성화 상태를 변경합니다.
	 *
	 * @param bEnabled true이면 QueryOnly, false이면 NoCollision으로 설정합니다.
	 */
	void SetBasketBlockersEnabled(bool bEnabled) const;

	/** 기존 블루프린트에 저장된 충돌 설정과 무관하게 바구니 트리거 설정을 강제로 맞춥니다. */
	void ConfigureBasketTriggerCollision(UBoxComponent* BasketTrigger) const;

	/** 재장전 대기 시간이 끝난 뒤 다시 작동 가능한 상태로 전환합니다. */
	void Rearm();

	/** 바구니에 떨어져 대기 중인 공이 발사 전까지 튀거나 밀려나지 않도록 이동을 잠급니다. */
	void LockBasketBallMovement(UPrimitiveComponent* BallComponent);

	/** 지정한 공 컴포넌트의 X/Y 이동 잠금을 원래 상태로 되돌립니다. */
	void RestoreLockedBasketBallMovement(UPrimitiveComponent* BallComponent);

	/** 대기 중 잠겨 있던 공을 발사 가능한 물리 상태로 되돌립니다. */
	void PrepareBasketBallForLaunch(UPrimitiveComponent* BallComponent);

	/** 바구니에 들어온 공을 중앙에 고정하고 튕김을 만드는 속도 성분을 정리합니다. */
	void StabilizeBasketBall(EDominoSeesawBasketSide BasketSide, UPrimitiveComponent* BallComponent);

	/** 지정한 바구니 안에서 공을 고정할 월드 위치를 반환합니다. */
	FVector GetBasketHoldLocation(EDominoSeesawBasketSide BasketSide) const;

	/** 지정한 바구니에서 반대편 바구니로 향하는 수평 월드 방향을 반환합니다. */
	FVector GetLaunchHorizontalDirection(EDominoSeesawBasketSide LaunchSide) const;

	/** 이 시소가 잠근 공 컴포넌트들의 물리 축 잠금 상태를 원래대로 되돌립니다. */
	void RestoreLockedBasketBallMovement();

	struct FLockedBasketBallMovementState
	{
		TWeakObjectPtr<UPrimitiveComponent> Component;
		uint8 OriginalDOFMode = 0;
		bool bOriginalLockTranslation = false;
		bool bOriginalLockRotation = false;
		bool bOriginalLockXTranslation = false;
		bool bOriginalLockYTranslation = false;
		bool bOriginalLockZTranslation = false;
		bool bOriginalLockXRotation = false;
		bool bOriginalLockYRotation = false;
		bool bOriginalLockZRotation = false;
	};

	/** 도미노 시뮬레이션 중인지 여부입니다. */
	bool bSimulationEnabled = false;

	/** 현재 다른 공을 발사할 수 있는 대기 상태인지 여부입니다. */
	bool bArmed = true;

	/** BeamRoot가 보간해 도달할 목표 Roll 각도입니다. */
	float TargetBeamRoll = 0.0f;

	/** 리셋 시 되돌아갈 BeginPlay 시점의 액터 트랜스폼입니다. */
	FTransform InitialTransform = FTransform::Identity;

	/** 바구니 충돌 박스를 마지막으로 활성화한 월드 시간입니다. */
	float LastBasketBlockerEnabledTime = -FLT_MAX;

	/** 왼쪽 바구니에 들어와 대기 중인 공 컴포넌트입니다. */
	TWeakObjectPtr<UPrimitiveComponent> LeftBasketBallComponent;

	/** 오른쪽 바구니에 들어와 대기 중인 공 컴포넌트입니다. */
	TWeakObjectPtr<UPrimitiveComponent> RightBasketBallComponent;

	/** 시소 작동 중 X/Y 이동을 잠근 공 컴포넌트들의 원래 물리 축 잠금 상태입니다. */
	TArray<FLockedBasketBallMovementState> LockedBasketBallMovementStates;

	/** Rearm 호출을 예약하는 타이머 핸들입니다. */
	FTimerHandle RearmTimerHandle;
};
