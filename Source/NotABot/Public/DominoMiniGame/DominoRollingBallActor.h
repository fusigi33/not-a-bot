#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoMiniGame/DominoSimulationObjectInterface.h"
#include "DominoRollingBallActor.generated.h"

class ADominoBlockActor;
class UStaticMeshComponent;

/**
 * 도미노에 부딪히면 충돌 방향으로 굴러가는 물리 공 액터입니다.
 */
UCLASS()
class NOTABOT_API ADominoRollingBallActor : public AActor, public IDominoSimulationObjectInterface
{
	GENERATED_BODY()

public:
	ADominoRollingBallActor();

	virtual void SetDominoSimulationEnabled_Implementation(bool bEnabled) override;
	virtual void ResetDominoSimulationObject_Implementation() override;

	/** 공의 비주얼과 충돌을 담당하는 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino Rolling Ball|Components")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	/** 기본 UE Sphere 메시 기준 반지름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Shape", meta = (ClampMin = "1.0"))
	float BallRadius = 25.0f;

	/** 도미노 충돌 시 공에 추가로 가할 수평 임펄스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Hit", meta = (ClampMin = "0.0"))
	float DominoHitImpulseStrength = 950.0f;

	/** 충돌한 도미노 속도가 이 값 이상일 때만 추가 임펄스를 적용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Hit", meta = (ClampMin = "0.0"))
	float MinimumDominoHitSpeed = 5.0f;

	/** 충돌 지점에서 살짝 띄워 끼임을 줄이는 위쪽 임펄스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Hit", meta = (ClampMin = "0.0"))
	float UpwardHitImpulseStrength = 40.0f;

	/** 같은 충돌에서 임펄스가 여러 번 적용되는 것을 막는 짧은 쿨다운입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Hit", meta = (ClampMin = "0.0"))
	float HitImpulseCooldown = 0.12f;

	/** true이면 ADominoBlockActor와의 충돌에만 반응합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Hit")
	bool bOnlyReactToDominoHits = true;

	/** 물리 질량입니다. 0 이하이면 메시 기본 질량을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Physics", meta = (ClampMin = "0.0"))
	float BallMassKg = 8.0f;

	/** 공의 선형 감쇠입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Physics", meta = (ClampMin = "0.0"))
	float LinearDamping = 0.08f;

	/** 공의 회전 감쇠입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Physics", meta = (ClampMin = "0.0"))
	float AngularDamping = 0.05f;

	/** 시뮬레이션 시작 시 공을 즉시 깨울지 여부입니다. false이면 도미노가 닿을 때 깨어납니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino Rolling Ball|Physics")
	bool bWakeOnSimulationStart = false;

	UFUNCTION(BlueprintCallable, Category = "Domino Rolling Ball")
	void SetBallPhysicsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Domino Rolling Ball")
	void ApplyDominoHitImpulse(const FVector& HitLocation, const FVector& IncomingVelocity);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void HandleBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyBallSettings() const;
	bool ShouldReactToHit(AActor* OtherActor, UPrimitiveComponent* OtherComp) const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino Rolling Ball")
	bool bSimulationEnabled = false;

	FTransform InitialTransform = FTransform::Identity;
	float LastHitImpulseTime = -FLT_MAX;
};
