#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoMiniGame/DominoSimulationObjectInterface.h"
#include "DominoPendulumActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * 도미노 미니게임용 진자 오브젝트입니다.
 *
 * 추는 가운데 최저점에서 멈춰 있는 상태로 시작하고,
 * 힘을 받으면 MaxLiftHeight 높이만큼 올라가는 진폭으로 왕복 운동합니다.
 */
UCLASS()
class NOTABOT_API ADominoPendulumActor : public AActor, public IDominoSimulationObjectInterface
{
	GENERATED_BODY()

public:
	ADominoPendulumActor();

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetDominoSimulationEnabled_Implementation(bool bEnabled) override;
	virtual void ResetDominoSimulationObject_Implementation() override;

	/** 진자 피벗입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Pendulum|Components")
	TObjectPtr<USceneComponent> PivotRoot;

	/** 피벗 비주얼 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Pendulum|Components")
	TObjectPtr<UStaticMeshComponent> PivotMesh;

	/** 줄/막대 비주얼 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Pendulum|Components")
	TObjectPtr<UStaticMeshComponent> ArmMesh;

	/** 충돌과 타격을 담당하는 추 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Pendulum|Components")
	TObjectPtr<UStaticMeshComponent> BobMesh;

	/** 피벗에서 추 중심까지의 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Motion", meta = (ClampMin = "1.0"))
	float PendulumLength = 250.0f;

	/** 힘을 받았을 때 추가 최저점에서 위로 올라가는 최대 높이입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Motion", meta = (ClampMin = "0.0"))
	float MaxLiftHeight = 120.0f;

	/** 한쪽 끝에서 반대쪽 끝까지 도달하는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Motion", meta = (ClampMin = "0.05"))
	float HalfSwingDuration = 0.75f;

	/** 시뮬레이션 시작 시 자동으로 흔들리게 할지 여부입니다. 기본값은 가운데 정지 상태입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Motion")
	bool bAutoStartWhenSimulationEnabled = false;

	/** 첫 충돌의 속도가 이 값 이상일 때 진자가 작동합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Motion", meta = (ClampMin = "0.0"))
	float MinimumActivationSpeed = 5.0f;

	/** 충돌 속도와 진자 로컬 X축의 부호가 반대로 매핑된 에셋 배치에서 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Motion")
	bool bInvertImpactDirection = true;

	/** 추가 물체를 칠 때 속도에 곱해 적용할 임펄스 배율입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Collision", meta = (ClampMin = "0.0"))
	float HitImpulseMultiplier = 35.0f;

	/** ArmMesh가 로컬 X축 방향으로 길게 만들어졌는지 여부입니다. false이면 로컬 Z축 방향으로 정렬합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Visual")
	bool bArmMeshUsesLocalXAxis = false;

	/** ArmMesh가 로컬 Z축 길이 메시일 때 적용할 로컬 X 두께 스케일입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Visual", meta = (ClampMin = "0.001"))
	float ArmMeshScaleX = 0.01f;

	/** ArmMesh가 로컬 Z축 길이 메시일 때 적용할 로컬 Y 두께 스케일입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Visual", meta = (ClampMin = "0.001"))
	float ArmMeshScaleY = 0.01f;

	/** ArmMesh 원본 메시의 길이입니다. UE 기본 Cylinder는 보통 100cm입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Visual", meta = (ClampMin = "1.0"))
	float ArmMeshReferenceLength = 100.0f;

	/** true이면 ArmMeshReferenceLength 대신 Static Mesh Bounds에서 실제 길이를 자동으로 읽습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Visual")
	bool bUseArmMeshBoundsForLength = true;

	/** ArmMesh 끝이 Bob 중심까지 들어가지 않도록 Bob 중심에서 피벗 방향으로 빼는 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Pendulum|Visual", meta = (ClampMin = "0.0"))
	float ArmBobCenterOffset = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Domino Pendulum")
	void StartSwing();

	/** 외부에서 진자에 힘을 가해 가운데 정지 상태에서 흔들리게 합니다. ForceDirectionSign은 양수/음수로 방향을 정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino Pendulum")
	void ApplyPendulumForce(float ForceDirectionSign = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Domino Pendulum")
	void StopSwing();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void HandleBobHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void UpdatePendulum(float NewSwingTime, bool bSweepBob);
	void UpdateArmVisual(const FVector& BobLocalLocation);
	FVector GetBobLocalLocation() const;
	float GetArmMeshSourceLength() const;
	float GetMaxSwingAngleRadians() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino Pendulum")
	bool bSwinging = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino Pendulum")
	bool bSimulationEnabled = false;

	FTransform InitialTransform = FTransform::Identity;
	float SwingTime = 0.0f;
	float SwingDirectionSign = 1.0f;
	FVector PreviousBobWorldLocation = FVector::ZeroVector;
	FVector CurrentBobVelocity = FVector::ZeroVector;
};
