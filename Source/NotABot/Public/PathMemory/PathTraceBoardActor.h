#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathTraceBoardActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USplineComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UBoxComponent;

// 스플라인(정답 경로) 상의 특정 지점에 대한 정보
USTRUCT(BlueprintType)
struct FPathCheckpoint
{
	GENERATED_BODY()

	// 실제 월드 좌표
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	// 스플라인 시작점으로부터의 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DistanceAlongSpline = 0.f;
};

UCLASS()
class NOTABOT_API APathTraceBoardActor : public AActor
{
	GENERATED_BODY()

public:
	APathTraceBoardActor();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> BoardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> AnswerSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneCaptureComponent2D> TopSceneCapture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Render")
	TObjectPtr<UTextureRenderTarget2D> BoardRenderTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> GoalTrigger;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	TObjectPtr<UBoxComponent> LeftWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	TObjectPtr<UBoxComponent> RightWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	TObjectPtr<UBoxComponent> TopWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	TObjectPtr<UBoxComponent> BottomWall;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Capture")
	float TopCaptureHeight = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Capture")
	float TopCapturePitch = -60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Scoring")
	float AllowedDeviation = 120.f;

	// 체크포인트를 스플라인 위에 얼마의 간격으로 배치할지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Scoring")
	float CheckpointSpacing = 120.f;

	// 플레이어가 체크포인트 반경 몇 언리얼 유닛 안에 들어와야 통과한 것으로 인정할지 결정하는 판정 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Scoring")
	float CheckpointReachRadius = 140.f;

	// 플레이어의 현재 진행 상황(거리)을 기준으로, 뒤로는 120, 앞으로는 220만큼의 범위 내에서만 플레이어의 위치를 스플라인에 매핑(투영)
	// 앞으로 더 많이 열어둔 것은 플레이어가 빠르게 전진하는 상황을 고려
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Scoring")
	float ProjectionSearchBackDistance = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Scoring")
	float ProjectionSearchForwardDistance = 220.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	float WallThickness = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	float WallHeight = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace|Wall")
	float WallInset = 0.f;

public:
	// 새로운 라운드의 궤적을 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category="PathTrace|Round")
	void SetupNewRoundSpline(const TArray<FVector>& NewPathPoints);
	
	void SetupCaptureTransform();

	FVector GetSplineStartLocation() const;
	FVector GetSplineEndLocation() const;
	float GetSplineLength() const;
	FVector GetLocationAtDistance(float Distance) const;
	float GetDistanceAlongSplineAtWorldLocation(const FVector& WorldLocation) const;

	// 전체 스플라인을 CheckpointSpacing 간격으로 나누어 FPathCheckpoint 배열을 만들어냅니다.
	void BuildCheckpoints(TArray<FPathCheckpoint>& OutCheckpoints) const;

	/**
	 * 플레이어의 실제 위치(WorldLocation)가 스플라인 경로 상에서 어느 정도 진행된 상태(Distance)인지 계산합니다.
	 * 
	 * @param WorldLocation 현재 검사하려는 점의 3D 월드 좌표
	 * @param PrevDistanceAlongSpline 플레이어의 이전 진행 거리
	 * @param SearchBackDistance 뒤쪽 탐색 범위
	 * @param SearchForwardDistance 앞쪽 탐색 범위
	 * @param SampleStep 탐색 범위 내에서 점을 몇 언리얼 유닛 간격으로 찍어가며 검사할지 결정하는 해상도
	 * @return 
	 */
	float FindConstrainedDistanceAlongSpline(
		const FVector& WorldLocation,
		float PrevDistanceAlongSpline,
		float SearchBackDistance,
		float SearchForwardDistance,
		float SampleStep = 20.f
	) const;
	
	void UpdateInvisibleWalls();
	
private:
	void SetupWallCollision(UBoxComponent* Wall);
};