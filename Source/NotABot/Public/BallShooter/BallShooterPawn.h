#pragma once

#include "CoreMinimal.h"
#include "BallShooter/BallShooterTypes.h"
#include "GameFramework/Pawn.h"
#include "BallShooterPawn.generated.h"

class ABallShooterGameManager;
class UInputAction;
class UInputMappingContext;
class UInstancedStaticMeshComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
struct FInputActionValue;

/** 폰이 목표에 도달했을 때 호출되는 델리게이트입니다. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallShooterPawnGoalSignature, ABallShooterPawn*, Pawn);
/** 폰의 라운드 결과가 확정되었을 때 호출되는 델리게이트입니다. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallShooterPawnFinishedSignature, ABallShooterPawn*, Pawn, EBallShooterRoundResult, Result);

/**
 * 플레이어 입력으로 발사 각도와 속도를 조절하는 조준 폰입니다.
 * 궤적 프리뷰를 갱신하고 공 발사를 트리거합니다.
 */
UCLASS()
class NOTABOT_API ABallShooterPawn : public APawn
{
	GENERATED_BODY()

public:
	/** 조준, 투사체, 궤적 프리뷰 컴포넌트를 초기화합니다. */
	ABallShooterPawn();
	
	/** 입력 매핑을 등록하고 궤적 프리뷰를 초기화합니다. */
	virtual void BeginPlay() override;

	/** 조준, 차지, 발사 후 상태를 매 프레임 업데이트합니다. */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * 라운드 결과 보고에 사용할 게임 매니저를 설정합니다.
	 * @param InGameManager 현재 폰의 상태를 관리할 매니저입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void SetGameManager(ABallShooterGameManager* InGameManager);

	/**
	 * 폰을 초기 위치로 되돌리고 조준 상태를 초기화합니다.
	 * @param SpawnTransform 시작 기준 트랜스폼입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void ResetAiming(const FTransform& SpawnTransform);

	/**
	 * 폰을 지정한 초기 속도로 발사합니다.
	 * @param InitialVelocity 폰에 적용할 초기 속도입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void LaunchBall(const FVector& InitialVelocity);

	/** 폰의 이동을 즉시 중지하고 내부 상태를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void StopBall();

	/** 목표 지점 도달을 통보하여 성공 처리합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void NotifyReachedGoal();

	/**
	 * 폰이 유효하게 움직일 수 있는 보드 경계를 설정합니다.
	 * @param InMinBounds 보드의 최소 월드 좌표입니다.
	 * @param InMaxBounds 보드의 최대 월드 좌표입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void SetBoardBounds(const FVector& InMinBounds, const FVector& InMaxBounds);

	/**
	 * 현재 조준 상태를 변경하고 시각 요소를 갱신합니다.
	 * @param NewState 새 조준 상태입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void SetAimState(EBallShooterAimState NewState);

	/** 현재 조준 방향 기준으로 궤적 프리뷰를 다시 생성합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Pawn")
	void RefreshTrajectory();

	/** 현재 차지로 계산된 발사 속도를 반환합니다. */
	UFUNCTION(BlueprintPure, Category="BallShooter|Pawn")
	float GetCurrentLaunchSpeed() const { return CurrentLaunchSpeed; }

	/** 현재 폰이 발사된 상태인지 반환합니다. */
	UFUNCTION(BlueprintPure, Category="BallShooter|Pawn")
	bool IsLaunched() const { return bLaunched; }

	/** 현재 투사체 속도를 반환합니다. */
	UFUNCTION(BlueprintPure, Category="BallShooter|Pawn")
	FVector GetCurrentVelocity() const;

	/** 폰이 목표에 도달했을 때 브로드캐스트됩니다. */
	UPROPERTY(BlueprintAssignable, Category="BallShooter|Events")
	FOnBallShooterPawnGoalSignature OnBallReachedGoal;

	/** 폰의 결과가 확정되었을 때 브로드캐스트됩니다. */
	UPROPERTY(BlueprintAssignable, Category="BallShooter|Events")
	FOnBallShooterPawnFinishedSignature OnBallFinished;

	/** 블루프린트에서 성공 연출을 구현할 때 호출됩니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="BallShooter|Events")
	void BP_OnBallReachedGoal();

	/**
	 * 블루프린트에서 실패 또는 도달 불가 연출을 구현할 때 호출됩니다.
	 * @param Result 폰의 최종 라운드 결과입니다.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="BallShooter|Events")
	void BP_OnBallBecameUnreachable(EBallShooterRoundResult Result);

protected:
	/** 빙의 시 입력 매핑을 다시 등록합니다. */
	virtual void PossessedBy(AController* NewController) override;

	/** 강화 입력 액션을 바인딩합니다. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 폰의 충돌과 투사체 이동을 담당하는 루트 구체 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** 폰의 비주얼을 표현하는 메시 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	/** 발사 방향과 위치의 기준이 되는 씬 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<USceneComponent> LaunchOrigin;

	/** 예측 궤적 점들을 인스턴스로 렌더링하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UInstancedStaticMeshComponent> TrajectoryInstances;

	/** 폰의 물리 기반 비행을 담당하는 투사체 이동 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 조준/차지 입력을 등록할 입력 매핑 컨텍스트입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/** 조준 축 입력 액션입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Input")
	TObjectPtr<UInputAction> AimInputAction;

	/** 차지 시작/발사 입력 액션입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Input")
	TObjectPtr<UInputAction> ChargeInputAction;

	/** 입력 매핑 컨텍스트 우선순위입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Input", meta=(ClampMin="0"))
	int32 MappingPriority = 10;

	/** 현재 좌우 조준 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Aim", meta=(ClampMin="-180.0", ClampMax="180.0"))
	float AimYawDegrees = 0.0f;

	/** 조준 가능한 최소 Yaw 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Aim", meta=(ClampMin="-180.0", ClampMax="180.0"))
	float MinAimYawDegrees = -70.0f;

	/** 조준 가능한 최대 Yaw 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Aim", meta=(ClampMin="-180.0", ClampMax="180.0"))
	float MaxAimYawDegrees = 70.0f;

	/** 초당 조준 회전 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Aim", meta=(ClampMin="0.0"))
	float AimYawSpeedDegrees = 80.0f;

	/** 발사 방향에 항상 적용되는 고정 피치 각도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Aim", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float LaunchPitchDegrees = 0.0f;

	/** 차지 시작 시 최소 발사 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Charge", meta=(ClampMin="0.0"))
	float MinLaunchSpeed = 900.0f;

	/** 차지 최대 시 발사 가능한 최고 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Charge", meta=(ClampMin="0.0"))
	float MaxLaunchSpeed = 2400.0f;

	/** 충돌 후 얼마나 많이 튕겨나갈지 결정하는 반발 계수입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Bounce", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BounceBounciness = 0.82f;

	/** 충돌 면을 따라 미끄러질 때 잃는 속도량입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Bounce", meta=(ClampMin="0.0"))
	float BounceFriction = 0.08f;

	/** 이 속도보다 느려지면 ProjectileMovement가 자체적으로 정지합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Bounce", meta=(ClampMin="0.0"))
	float BounceVelocityStopThreshold = 5.0f;

	/** 최소 속도에서 최대 속도까지 차지하는 데 걸리는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Charge", meta=(ClampMin="0.01"))
	float ChargeDuration = 1.4f;

	/** 차지 시 속도가 왕복 변화하는지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Charge")
	bool bPingPongCharge = true;

	/** 궤적 예측에 사용할 최대 시뮬레이션 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Trajectory", meta=(ClampMin="0.1"))
	float TrajectorySimTime = 3.0f;

	/** 궤적 시뮬레이션 빈도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Trajectory", meta=(ClampMin="4.0"))
	float TrajectorySimFrequency = 24.0f;

	/** 궤적 점 메시의 스케일입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Trajectory")
	float TrajectoryPointScale = 0.1f;

	/** 화면에 보이는 궤적 점들을 Z축으로 추가 보정하는 값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Trajectory")
	float TrajectoryZOffset = -10.0f;

	/** 궤적 예측에 사용할 충돌 채널입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Trajectory")
	TEnumAsByte<ECollisionChannel> TrajectoryCollisionChannel = ECC_Visibility;

	/** 공이 멈춘 것으로 간주할 최소 속도 임계값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rules", meta=(ClampMin="0.0"))
	float StopSpeedThreshold = 80.0f;

	/** 낮은 속도가 유지되면 실패 처리하기까지의 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rules", meta=(ClampMin="0.0"))
	float TimeBelowStopSpeedToFail = 1.5f;

	/** 공 비행이 허용되는 최대 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rules", meta=(ClampMin="0.1"))
	float MaxFlightTime = 12.0f;

	/** 공의 위치가 벗어나면 실패 처리할 최소 보드 경계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rules")
	FVector BoardMinBounds = FVector(-1500.0f, -1500.0f, -400.0f);

	/** 공의 위치가 벗어나면 실패 처리할 최대 보드 경계입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rules")
	FVector BoardMaxBounds = FVector(1500.0f, 1500.0f, 900.0f);

	/** 허용되는 최대 바운스 횟수입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Rules", meta=(ClampMin="0"))
	int32 MaxBounceCount = 16;

	/** 현재 폰의 조준 상태입니다. */
	UPROPERTY(BlueprintReadOnly, Category="BallShooter|State")
	EBallShooterAimState AimState = EBallShooterAimState::Aiming;

private:
	/** 폰의 결과를 보고할 게임 매니저 참조입니다. */
	UPROPERTY(Transient)
	TObjectPtr<ABallShooterGameManager> GameManager;

	/** 현재 입력 중인 조준 축 값입니다. */
	float AimInputAxis = 0.0f;
	/** 차지에 사용되는 누적 시간입니다. */
	float ChargeTime = 0.0f;
	/** 현재 계산된 발사 속도입니다. */
	float CurrentLaunchSpeed = 900.0f;
	/** 폰이 현재 발사되었는지 여부입니다. */
	bool bLaunched = false;
	/** 폰의 결과가 이미 확정되었는지 여부입니다. */
	bool bFinished = false;
	/** 현재 발사 이후 누적 비행 시간입니다. */
	float FlightTime = 0.0f;
	/** 정지 임계 속도 이하로 머문 누적 시간입니다. */
	float TimeBelowStopSpeed = 0.0f;
	/** 현재까지 발생한 바운스 횟수입니다. */
	int32 BounceCount = 0;
	/** 너무 짧은 예측 경로에 대한 경고 로그를 이미 남겼는지 여부입니다. */
	bool bLoggedShortTrajectoryWarning = false;

	/** 조준 축 입력을 받아 저장합니다. */
	void HandleAimInput(const FInputActionValue& Value);
	/** 조준 입력 종료 시 축 값을 초기화합니다. */
	void HandleAimInputCompleted(const FInputActionValue& Value);
	/** 차지 시작 입력을 처리합니다. */
	void HandleChargeStarted(const FInputActionValue& Value);
	/** 차지 종료 입력 시 공을 발사합니다. */
	void HandleChargeCompleted(const FInputActionValue& Value);

	/** 조준 입력에 따라 발사 방향을 갱신합니다. */
	void UpdateAim(float DeltaSeconds);
	/** 시간 경과에 따라 현재 발사 속도를 갱신합니다. */
	void UpdateCharge(float DeltaSeconds);
	/**
	 * 투사체가 충돌로 반사될 때 바운스 횟수를 갱신합니다.
	 * @param ImpactResult 반사 시 충돌 결과입니다.
	 * @param ImpactVelocity 충돌 직전 속도입니다.
	 */
	UFUNCTION()
	void HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	/** 궤적 프리뷰 메시 표시 여부를 전환합니다. */
	void SetTrajectoryVisible(bool bVisible);
	/** 현재 발사 방향과 속도로 실제 초기 속도를 구성합니다. */
	FVector BuildLaunchVelocity(float Speed) const;
	/** 현재 공 충돌 설정을 기준으로 프리뷰 경로에 사용할 오브젝트 타입 목록을 구성합니다. */
	TArray<TEnumAsByte<EObjectTypeQuery>> BuildTrajectoryObjectTypes() const;
	/** 기존에 생성된 궤적 프리뷰 인스턴스를 제거합니다. */
	void ClearTrajectory();
	/** 현재 로컬 플레이어 기준으로 입력 매핑을 등록합니다. */
	void RegisterInputMappingContext() const;
	/**
	 * 폰의 최종 결과를 확정하고 관련 이벤트를 전파합니다.
	 * @param Result 폰의 최종 라운드 결과입니다.
	 */
	void FinishBall(EBallShooterRoundResult Result);
	/** 폰이 보드 경계를 벗어났는지 검사합니다. */
	bool IsOutsideBoardBounds() const;
};
