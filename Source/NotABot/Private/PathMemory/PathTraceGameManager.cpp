#include "PathMemory/PathTraceGameManager.h"
#include "PathMemory/PathTraceBoardActor.h"
#include "PathMemory/LightTrailActor.h"
#include "PathMemory/PathTraceCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

APathTraceGameManager::APathTraceGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APathTraceGameManager::BeginPlay()
{
	Super::BeginPlay();

	// 컨트롤러 캐싱
	PC = UGameplayStatics::GetPlayerController(this, 0);

	// 보드판의 도착 지점(GoalTrigger)에 누군가 닿으면(Overlap), HandleGoalReached 함수를 즉시 실행하도록 연결
	if (BoardActor && BoardActor->GoalTrigger)
	{
		BoardActor->GoalTrigger->OnComponentBeginOverlap.AddDynamic(
			this,
			&APathTraceGameManager::HandleGoalReached
		);
	}

	// 빛줄기(LightTrailActor)가 목표 지점까지 다 날아가서 궤적 그리기가 끝나면, HandleLightTrailFinished 함수를 실행하도록 연결
	if (LightTrailActor)
	{
		LightTrailActor->OnTrailFinished.AddDynamic(this, &APathTraceGameManager::HandleLightTrailFinished);
	}
}

void APathTraceGameManager::StartMiniGame()
{
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(this, 0);
	}

	if (!BoardActor || !LightTrailActor || !PlayerCharacter || !PC)
	{
		return;
	}

	CurrentState = EPathTraceState::ShowingAnswer;

	PlayerCharacter->SetCanPlayerMove(false);
	PlayerCharacter->ResetRecordedPath();

	// 먼저 보드 상단 perspective scene capture 화면을 보여주는 흐름은
	// 기존 UMG에서 RenderTarget 머티리얼을 띄워두면 됨
	// 여기서는 빛줄기만 재생
	LightTrailActor->StartTrail(BoardActor);
}

void APathTraceGameManager::HandleLightTrailFinished()
{
	if (bWaitingForRecordedPlayerPath)
	{
		bWaitingForRecordedPlayerPath = false;
		OnRecordedPlayerPathFinished.Broadcast(bRecordedPlayerPathSuccess);
		return;
	}

	if (AutoPossessDelaySeconds <= 0.f)
	{
		EnterPlayerTurn();
		return;
	}

	FTimerHandle AutoPossessTimerHandle;
	GetWorldTimerManager().SetTimer(
		AutoPossessTimerHandle,
		this,
		&APathTraceGameManager::EnterPlayerTurn,
		AutoPossessDelaySeconds,
		false
	);
}

void APathTraceGameManager::EnterPlayerTurn()
{
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(this, 0);
	}

	if (!PC || !PlayerCharacter)
	{
		return;
	}

	CurrentState = EPathTraceState::PlayerTurn;

	if (PC->GetPawn() != PlayerCharacter)
	{
		PC->Possess(PlayerCharacter);
	}

	PC->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);

	PC->SetViewTargetWithBlend(PlayerCharacter, 1.0f);
	PlayerCharacter->ResetRecordedPath();
	PlayerCharacter->SetCanPlayerMove(true);
}

void APathTraceGameManager::HandleGoalReached(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (CurrentState != EPathTraceState::PlayerTurn)
	{
		return;
	}

	if (OtherActor != PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->SetCanPlayerMove(false);

	const float Accuracy = EvaluateAccuracyPercent();
	const bool bSuccess = Accuracy >= RequiredAccuracyPercent;

	FinishMiniGame(bSuccess, Accuracy);
}

float APathTraceGameManager::EvaluateAccuracyPercent() const
{
	// 안전 검사 및 초기화
	if (!BoardActor || !PlayerCharacter)
	{
		return 0.f;
	}

	const TArray<FVector>& Path = PlayerCharacter->GetRecordedPath();
	if (Path.Num() <= 1)
	{
		return 0.f;
	}

	float TotalWeight = 0.f;
	float MatchedWeight = 0.f;

	const float SplineLength = BoardActor->GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	float PrevProjectedDist = 0.f;
	bool bHasPrevProjection = false;

	for (int32 i = 0; i < Path.Num(); ++i)
	{
		const FVector& Sample = Path[i];

		const float ProjectedDist = BoardActor->GetDistanceAlongSplineAtWorldLocation(Sample);
		const FVector ClosestOnSpline = BoardActor->GetLocationAtDistance(ProjectedDist);

		const float DistToSpline2D = FVector::Dist2D(Sample, ClosestOnSpline);

		float SegmentWeight = 1.f;
		if (i > 0)
		{
			SegmentWeight = FMath::Max(1.f, FVector::Dist2D(Path[i - 1], Sample));
		}

		TotalWeight += SegmentWeight;

		// 스플라인에 충분히 가까워야 함
		const bool bNearEnough = DistToSpline2D <= BoardActor->AllowedDeviation;

		// 진행 방향이 너무 거꾸로면 감점
		bool bForwardEnough = true;
		if (bHasPrevProjection)
		{
			const float DeltaAlongSpline = ProjectedDist - PrevProjectedDist;
			bForwardEnough = DeltaAlongSpline >= -40.f; // 약간의 흔들림 허용
		}

		if (bNearEnough && bForwardEnough)
		{
			// 충분히 가까운 거리는 완전 일치로 처리하고, 그 밖은 허용 거리까지 점진적으로 감점
			const float PerfectDeviation = FMath::Clamp(PerfectAccuracyDeviation, 0.f, BoardActor->AllowedDeviation);
			const float ScoredDeviationRange = FMath::Max(BoardActor->AllowedDeviation - PerfectDeviation, KINDA_SMALL_NUMBER);
			const float Closeness = DistToSpline2D <= PerfectDeviation
				? 1.f
				: 1.f - FMath::Clamp((DistToSpline2D - PerfectDeviation) / ScoredDeviationRange, 0.f, 1.f);
			MatchedWeight += SegmentWeight * Closeness;
		}

		PrevProjectedDist = ProjectedDist;
		bHasPrevProjection = true;
	}

	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return (MatchedWeight / TotalWeight) * 100.f;
}

void APathTraceGameManager::ShowAnswerPath(float LifeTime) const
{
	if (!BoardActor)
	{
		return;
	}

	BoardActor->ShowAnswerPath(LifeTime);
}

void APathTraceGameManager::ShowRecordedPlayerPath(float Duration)
{
	if (!LightTrailActor || !PlayerCharacter)
	{
		bWaitingForRecordedPlayerPath = false;
		bRecordedPlayerPathSuccess = false;
		OnRecordedPlayerPathFinished.Broadcast(false);
		return;
	}

	const TArray<FVector>& RecordedPath = PlayerCharacter->GetRecordedPath();
	if (RecordedPath.Num() < 2)
	{
		bWaitingForRecordedPlayerPath = false;
		bRecordedPlayerPathSuccess = false;
		OnRecordedPlayerPathFinished.Broadcast(false);
		return;
	}

	const float AccuracyPercent = EvaluateAccuracyPercent();
	bRecordedPlayerPathSuccess = AccuracyPercent >= RequiredAccuracyPercent;
	bWaitingForRecordedPlayerPath = true;
	LightTrailActor->StartTrailFromPath(RecordedPath, Duration, true);
}

void APathTraceGameManager::FinishMiniGame(bool bSuccess, float AccuracyPercent)
{
	CurrentState = EPathTraceState::Result;
	OnPathTraceFinished.Broadcast(bSuccess, AccuracyPercent);
}
