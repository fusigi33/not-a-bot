#include "PathMemory/PathTraceBoardActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "DrawDebugHelpers.h"

APathTraceBoardActor::APathTraceBoardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(Root);

	AnswerSpline = CreateDefaultSubobject<USplineComponent>(TEXT("AnswerSpline"));
	AnswerSpline->SetupAttachment(Root);

	TopSceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("TopSceneCapture"));
	TopSceneCapture->SetupAttachment(Root);
	TopSceneCapture->bCaptureEveryFrame = true;
	TopSceneCapture->bCaptureOnMovement = false;

	GoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTrigger"));
	GoalTrigger->SetupAttachment(Root);
	GoalTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GoalTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	GoalTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	LeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(BoardMesh);

	RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(BoardMesh);

	TopWall = CreateDefaultSubobject<UBoxComponent>(TEXT("TopWall"));
	TopWall->SetupAttachment(BoardMesh);

	BottomWall = CreateDefaultSubobject<UBoxComponent>(TEXT("BottomWall"));
	BottomWall->SetupAttachment(BoardMesh);

	SetupWallCollision(LeftWall);
	SetupWallCollision(RightWall);
	SetupWallCollision(TopWall);
	SetupWallCollision(BottomWall);
}

void APathTraceBoardActor::SetupWallCollision(UBoxComponent* Wall)
{
	if (!Wall)
	{
		return;
	}

	Wall->SetHiddenInGame(true);
	Wall->SetVisibility(true);

	Wall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Wall->SetCollisionObjectType(ECC_WorldStatic);
	Wall->SetCollisionResponseToAllChannels(ECR_Ignore);
	Wall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void APathTraceBoardActor::BeginPlay()
{
	Super::BeginPlay();

	if (BoardRenderTarget)
	{
		TopSceneCapture->TextureTarget = BoardRenderTarget;
	}

	SetupCaptureTransform();
	UpdateInvisibleWalls();

	const FVector EndLoc = GetSplineEndLocation();
	
	// 스플라인의 끝점은 보통 바닥(Z=0 부근)에 딱 붙어있기 때문에
	// 트리거 박스를 살짝 위로 띄워 올려 플레이어 캐릭터의 몸통과 자연스럽게 충돌(Overlap)할 수 있도록 높이를 맞춰줌
	GoalTrigger->SetWorldLocation(EndLoc + FVector(0.f, 0.f, 60.f));
}

void APathTraceBoardActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateInvisibleWalls();
}

void APathTraceBoardActor::SetupNewRoundSpline(const TArray<FVector>& NewPathPoints)
{
	if (!AnswerSpline || NewPathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("경로를 만들려면 최소 2개 이상의 점이 필요합니다."));
		return;
	}

	// 기존에 있던 스플라인 점들을 모두 지움
	AnswerSpline->ClearSplinePoints();

	// 전달 받은 새로운 점들을 순서대로 스플라인에 추가
	for (const FVector& Point : NewPathPoints)
	{
		// 공간 기준을 World로 설정하여 실제 월드 좌표계에 점을 생성
		// 세 번째 인자 'false'는 점을 하나 추가할 때마다 스플라인을 갱신하지 않겠다는 뜻 (최적화)
		AnswerSpline->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
	}

	// 점 추가가 모두 끝난 뒤 한 번에 스플라인 곡선을 업데이트(계산)
	AnswerSpline->UpdateSpline();

	// 스플라인 모양이 바뀌었으므로, 도착 지점(GoalTrigger)도 새로운 끝점으로 옮김
	const FVector EndLoc = GetSplineEndLocation();
	GoalTrigger->SetWorldLocation(EndLoc + FVector(0.f, 0.f, 60.f));
}

void APathTraceBoardActor::ShowAnswerPath(float LifeTime) const
{
	if (!GetWorld() || !AnswerSpline)
	{
		return;
	}

	const float SplineLength = GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float DrawLifeTime = LifeTime >= 0.f ? LifeTime : ReviewPathLifeTime;
	const float SampleStep = FMath::Max(5.f, ReviewSplineSampleDistance);
	FVector PrevLocation = GetLocationAtDistance(0.f) + FVector::UpVector * ReviewPathZOffset;

	for (float Distance = SampleStep; Distance < SplineLength; Distance += SampleStep)
	{
		const FVector CurrentLocation = GetLocationAtDistance(Distance) + FVector::UpVector * ReviewPathZOffset;
		DrawDebugLine(GetWorld(), PrevLocation, CurrentLocation, AnswerReviewPathColor, false, DrawLifeTime, 0, ReviewPathThickness);
		PrevLocation = CurrentLocation;
	}

	const FVector EndLocation = GetLocationAtDistance(SplineLength) + FVector::UpVector * ReviewPathZOffset;
	DrawDebugLine(GetWorld(), PrevLocation, EndLocation, AnswerReviewPathColor, false, DrawLifeTime, 0, ReviewPathThickness);
}

void APathTraceBoardActor::ShowPlayerPath(const TArray<FVector>& PlayerPath, float LifeTime)
{
	if (!GetWorld())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PlayerPathReplayTimerHandle);

	ReplayPlayerPath = PlayerPath;
	NextReplayPathIndex = 1;
	CurrentPlayerPathReplayLifeTime = LifeTime >= 0.f ? LifeTime : ReviewPathLifeTime;

	if (ReplayPlayerPath.Num() < 2)
	{
		return;
	}

	DrawNextPlayerPathSegment();

	if (NextReplayPathIndex < ReplayPlayerPath.Num())
	{
		const int32 SegmentCount = ReplayPlayerPath.Num() - 1;
		const float ReplayInterval = FMath::Max(0.01f, PlayerPathReplayDuration / SegmentCount);

		GetWorldTimerManager().SetTimer(
			PlayerPathReplayTimerHandle,
			this,
			&APathTraceBoardActor::DrawNextPlayerPathSegment,
			ReplayInterval,
			true
		);
	}
}

void APathTraceBoardActor::DrawNextPlayerPathSegment()
{
	if (!GetWorld() || NextReplayPathIndex <= 0 || NextReplayPathIndex >= ReplayPlayerPath.Num())
	{
		if (GetWorld())
		{
			GetWorldTimerManager().ClearTimer(PlayerPathReplayTimerHandle);
		}
		return;
	}

	const FVector PathOffset = FVector::UpVector * (ReviewPathZOffset + 8.f);

	DrawDebugLine(
		GetWorld(),
		ReplayPlayerPath[NextReplayPathIndex - 1] + PathOffset,
		ReplayPlayerPath[NextReplayPathIndex] + PathOffset,
		PlayerReviewPathColor,
		false,
		CurrentPlayerPathReplayLifeTime,
		0,
		ReviewPathThickness
	);

	++NextReplayPathIndex;

	if (NextReplayPathIndex >= ReplayPlayerPath.Num())
	{
		GetWorldTimerManager().ClearTimer(PlayerPathReplayTimerHandle);
	}
}

void APathTraceBoardActor::SetupCaptureTransform()
{
	// const FVector Origin = BoardMesh ? BoardMesh->GetComponentLocation() : GetActorLocation();
	// TopSceneCapture->SetWorldLocation(Origin + FVector(0.f, 0.f, TopCaptureHeight));
	// TopSceneCapture->SetWorldRotation(FRotator(TopCapturePitch, 0.f, 0.f));
	// TopSceneCapture->FOVAngle = 55.f;
}

FVector APathTraceBoardActor::GetSplineStartLocation() const
{
	return AnswerSpline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
}

FVector APathTraceBoardActor::GetSplineEndLocation() const
{
	return AnswerSpline->GetLocationAtDistanceAlongSpline(
		AnswerSpline->GetSplineLength(),
		ESplineCoordinateSpace::World
	);
}

float APathTraceBoardActor::GetSplineLength() const
{
	return AnswerSpline->GetSplineLength();
}

FVector APathTraceBoardActor::GetLocationAtDistance(float Distance) const
{
	return AnswerSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float APathTraceBoardActor::GetLinearPathLength() const
{
	if (!AnswerSpline)
	{
		return 0.f;
	}

	const int32 PointCount = AnswerSpline->GetNumberOfSplinePoints();
	if (PointCount < 2)
	{
		return 0.f;
	}

	float TotalLength = 0.f;
	FVector PrevLocation = AnswerSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

	for (int32 PointIndex = 1; PointIndex < PointCount; ++PointIndex)
	{
		const FVector CurrentLocation = AnswerSpline->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
		TotalLength += FVector::Distance(PrevLocation, CurrentLocation);
		PrevLocation = CurrentLocation;
	}

	return TotalLength;
}

FVector APathTraceBoardActor::GetLinearLocationAtDistance(float Distance) const
{
	if (!AnswerSpline)
	{
		return GetActorLocation();
	}

	const int32 PointCount = AnswerSpline->GetNumberOfSplinePoints();
	if (PointCount == 0)
	{
		return GetActorLocation();
	}

	FVector PrevLocation = AnswerSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	if (PointCount == 1 || Distance <= 0.f)
	{
		return PrevLocation;
	}

	float RemainingDistance = Distance;

	for (int32 PointIndex = 1; PointIndex < PointCount; ++PointIndex)
	{
		const FVector CurrentLocation = AnswerSpline->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
		const float SegmentLength = FVector::Distance(PrevLocation, CurrentLocation);

		if (SegmentLength > KINDA_SMALL_NUMBER)
		{
			if (RemainingDistance <= SegmentLength)
			{
				return FMath::Lerp(PrevLocation, CurrentLocation, RemainingDistance / SegmentLength);
			}

			RemainingDistance -= SegmentLength;
		}

		PrevLocation = CurrentLocation;
	}

	return PrevLocation;
}

float APathTraceBoardActor::GetDistanceAlongSplineAtWorldLocation(const FVector& WorldLocation) const
{
	return AnswerSpline->GetDistanceAlongSplineAtLocation(WorldLocation, ESplineCoordinateSpace::World);
}

void APathTraceBoardActor::BuildCheckpoints(TArray<FPathCheckpoint>& OutCheckpoints) const
{
	// 배열 비우기
	OutCheckpoints.Reset();

	// 경로가 존재하지 않을 경우 함수 종료
	const float SplineLength = GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	/*
	 * 생성할 체크포인트의 개수(Count) 계산
	 * SplineLength / CheckpointSpacing: 전체 경로 길이를 설정한 간격으로 나눔
	 * FMath::CeilToInt(...): 이 값을 올림(Ceiling)하여 정수로 만듦
	 * + 1: 9개의 구간을 만들기 위해서는 양 끝을 포함해 총 10개의 점(말뚝)이 필요하므로 1을 더함
	 * FMath::Max(2, ...): 아무리 경로가 짧아도 최소한 '시작점'과 '끝점' 2개의 체크포인트는 반드시 보장하는 안전장치
	 */
	const int32 Count = FMath::Max(2, FMath::CeilToInt(SplineLength / CheckpointSpacing) + 1);

	for (int32 i = 0; i < Count; ++i)
	{
		// 0.0부터 1.0 사이의 진행 비율(Percentage)
		const float Alpha = (Count == 1) ? 0.f : static_cast<float>(i) / (Count - 1);
		// 해당 체크포인트가 스플라인 시작점으로부터 얼마나 떨어져 있는지
		const float Dist = Alpha * SplineLength;

		FPathCheckpoint CP;
		// : 방금 구한 거리 값
		CP.DistanceAlongSpline = Dist;
		// 해당 거리 위치의 실제 3D 월드 좌표
		CP.WorldLocation = GetLocationAtDistance(Dist);

		OutCheckpoints.Add(CP);
	}
}

float APathTraceBoardActor::FindConstrainedDistanceAlongSpline(
	const FVector& WorldLocation,
	float PrevDistanceAlongSpline,
	float SearchBackDistance,
	float SearchForwardDistance,
	float SampleStep
) const
{
	// 경로가 존재하지 않으면 종료
	const float SplineLength = GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	
	const float MinDist = FMath::Clamp(PrevDistanceAlongSpline - SearchBackDistance, 0.f, SplineLength);
	const float MaxDist = FMath::Clamp(PrevDistanceAlongSpline + SearchForwardDistance, 0.f, SplineLength);

	// 첫 샘플이거나 탐색 구간이 이상하면 전체 최근접 사용
	if (MaxDist <= MinDist + KINDA_SMALL_NUMBER)
	{
		return GetDistanceAlongSplineAtWorldLocation(WorldLocation);
	}

	float BestDist = MinDist;
	float BestSq = TNumericLimits<float>::Max();

	for (float Dist = MinDist; Dist <= MaxDist; Dist += SampleStep)
	{
		const FVector SplineLoc = GetLocationAtDistance(Dist);
		const float Sq = FVector::DistSquared2D(WorldLocation, SplineLoc);

		if (Sq < BestSq)
		{
			BestSq = Sq;
			BestDist = Dist;
		}
	}

	// 마지막 끝점도 체크
	{
		const FVector SplineLoc = GetLocationAtDistance(MaxDist);
		const float Sq = FVector::DistSquared2D(WorldLocation, SplineLoc);

		if (Sq < BestSq)
		{
			// BestSq = Sq;
			BestDist = MaxDist;
		}
	}

	return BestDist;
}

void APathTraceBoardActor::UpdateInvisibleWalls()
{
	if (!BoardMesh || !LeftWall || !RightWall || !TopWall || !BottomWall)
	{
		return;
	}

	const FBoxSphereBounds LocalBounds = BoardMesh->CalcLocalBounds();
	const FVector Origin = LocalBounds.Origin;
	const FVector Extent = LocalBounds.BoxExtent;

	const float HalfX = Extent.X - WallInset;
	const float HalfY = Extent.Y - WallInset;
	const float HalfZ = WallHeight * 0.5f;

	if (HalfX <= 0.f || HalfY <= 0.f)
	{
		return;
	}

	// Plane 기준 로컬 바운드에 맞춰 벽 배치
	LeftWall->SetRelativeLocation(FVector(Origin.X - HalfX - WallThickness * 0.5f, Origin.Y, Origin.Z + HalfZ));
	LeftWall->SetBoxExtent(FVector(WallThickness * 0.5f, HalfY, HalfZ));

	RightWall->SetRelativeLocation(FVector(Origin.X + HalfX + WallThickness * 0.5f, Origin.Y, Origin.Z + HalfZ));
	RightWall->SetBoxExtent(FVector(WallThickness * 0.5f, HalfY, HalfZ));

	TopWall->SetRelativeLocation(FVector(Origin.X, Origin.Y + HalfY + WallThickness * 0.5f, Origin.Z + HalfZ));
	TopWall->SetBoxExtent(FVector(HalfX, WallThickness * 0.5f, HalfZ));

	BottomWall->SetRelativeLocation(FVector(Origin.X, Origin.Y - HalfY - WallThickness * 0.5f, Origin.Z + HalfZ));
	BottomWall->SetBoxExtent(FVector(HalfX, WallThickness * 0.5f, HalfZ));
}
