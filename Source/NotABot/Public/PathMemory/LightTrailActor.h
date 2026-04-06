#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightTrailActor.generated.h"

class UNiagaraSystem;
class USceneComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class APathTraceBoardActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLightTrailFinished);

UCLASS()
class NOTABOT_API ALightTrailActor : public AActor
{
	GENERATED_BODY()

public:
	ALightTrailActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	// 빛의 궤적이 출발점부터 도착점까지 이동하는 데 걸리는 총 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PathTrace")
	float MoveDuration = 2.5f;

	// 궤적이 끝까지 이동을 마쳤을 때, 다른 시스템에 신호를 보내기 위한 이벤트 알림 장치
	UPROPERTY(BlueprintAssignable)
	FOnLightTrailFinished OnTrailFinished;

private:
	UPROPERTY()
	TObjectPtr<APathTraceBoardActor> Board;

	float ElapsedTime = 0.f;
	bool bPlaying = false;

public:
	void StartTrail(APathTraceBoardActor* InBoard);
	
protected:
	// 나이아가라 컴포넌트 (선 시각화용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> TrailNiagaraComponent;

	// 에디터에서 지정할 리본 파티클 시스템 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UNiagaraSystem> TrailSystemAsset;
};