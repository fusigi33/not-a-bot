#include "PathMemory/LightTrailActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PathMemory/PathTraceBoardActor.h"
#include "NiagaraComponent.h"

ALightTrailActor::ALightTrailActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(Root);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	TrailNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailNiagaraComponent"));
	TrailNiagaraComponent->SetupAttachment(RootComponent);
	TrailNiagaraComponent->bAutoActivate = false;
}

void ALightTrailActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALightTrailActor::StartTrail(APathTraceBoardActor* InBoard)
{
	if (!InBoard)
	{
		return;
	}

	Board = InBoard;
	ElapsedTime = 0.f;
	ActiveMoveDuration = MoveDuration;
	bUsingReplayPath = false;
	bBroadcastWhenFinished = true;
	bPlaying = (Board != nullptr);
	ReplayPathPoints.Reset();

	SetActorLocation(Board->GetSplineStartLocation());
	
	VisualMesh->SetHiddenInGame(false); // 구체 보이기

	// 나이아가라 시스템 에셋이 설정되어 있다면 활성화
	if (TrailSystemAsset)
	{
		TrailNiagaraComponent->SetAsset(TrailSystemAsset);
		TrailNiagaraComponent->Activate(true); // Reset 활성화
	}
}

void ALightTrailActor::StartTrailFromPath(const TArray<FVector>& PathPoints, float Duration, bool bShouldBroadcastWhenFinished)
{
	if (PathPoints.Num() < 2)
	{
		return;
	}

	Board = nullptr;
	ElapsedTime = 0.f;
	ActiveMoveDuration = Duration > 0.f ? Duration : PlayerPathMoveDuration;
	bUsingReplayPath = true;
	bBroadcastWhenFinished = bShouldBroadcastWhenFinished;
	bPlaying = true;
	ReplayPathPoints = PathPoints;

	SetActorLocation(ReplayPathPoints[0]);
	VisualMesh->SetHiddenInGame(false);

	if (TrailSystemAsset)
	{
		TrailNiagaraComponent->SetAsset(TrailSystemAsset);
		TrailNiagaraComponent->Activate(true);
	}
}

FVector ALightTrailActor::GetReplayPathLocation(float Alpha) const
{
	if (ReplayPathPoints.Num() == 0)
	{
		return GetActorLocation();
	}

	if (ReplayPathPoints.Num() == 1)
	{
		return ReplayPathPoints[0];
	}

	const float ClampedAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
	const float ScaledIndex = ClampedAlpha * static_cast<float>(ReplayPathPoints.Num() - 1);
	const int32 Index1 = FMath::Clamp(FMath::FloorToInt(ScaledIndex), 0, ReplayPathPoints.Num() - 2);
	const int32 Index2 = Index1 + 1;
	const float LocalAlpha = ScaledIndex - static_cast<float>(Index1);

	if (RecordedPathMode == ELightTrailPathMode::Curve)
	{
		const int32 Index0 = FMath::Max(Index1 - 1, 0);
		const int32 Index3 = FMath::Min(Index2 + 1, ReplayPathPoints.Num() - 1);

		const FVector& P0 = ReplayPathPoints[Index0];
		const FVector& P1 = ReplayPathPoints[Index1];
		const FVector& P2 = ReplayPathPoints[Index2];
		const FVector& P3 = ReplayPathPoints[Index3];
		const float T = LocalAlpha;
		const float T2 = T * T;
		const float T3 = T2 * T;

		return (
			(2.f * P1) +
			(-P0 + P2) * T +
			(2.f * P0 - 5.f * P1 + 4.f * P2 - P3) * T2 +
			(-P0 + 3.f * P1 - 3.f * P2 + P3) * T3
		) * 0.5f;
	}

	return FMath::Lerp(ReplayPathPoints[Index1], ReplayPathPoints[Index2], LocalAlpha);
}

void ALightTrailActor::ClearTrail()
{
	bPlaying = false;
	Board = nullptr;
	ElapsedTime = 0.f;
	ActiveMoveDuration = 0.f;
	bUsingReplayPath = false;
	ReplayPathPoints.Reset();
	VisualMesh->SetHiddenInGame(true);

	if (TrailNiagaraComponent)
	{
		TrailNiagaraComponent->DeactivateImmediate();
		TrailNiagaraComponent->ResetSystem();
	}
}

void ALightTrailActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPlaying)
	{
		return;
	}

	const float TrailLength = bUsingReplayPath
		? 1.f
		: (Board
			? (AnswerPathMode == ELightTrailPathMode::Linear ? Board->GetLinearPathLength() : Board->GetSplineLength())
			: 0.f);

	if (TrailLength <= KINDA_SMALL_NUMBER)
	{
		bPlaying = false;
		VisualMesh->SetHiddenInGame(true);
		TrailNiagaraComponent->Deactivate();
		return;
	}

	// 알파 계산
	ElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedTime / FMath::Max(ActiveMoveDuration, KINDA_SMALL_NUMBER), 0.f, 1.f);

	// 위치 업데이트
	const float Dist = Alpha * TrailLength;
	const FVector NewLoc = bUsingReplayPath
		? GetReplayPathLocation(Alpha)
		: (AnswerPathMode == ELightTrailPathMode::Linear ? Board->GetLinearLocationAtDistance(Dist) : Board->GetLocationAtDistance(Dist));
	SetActorLocation(NewLoc);

	// 목적지 도착했을 때
	if (Alpha >= 1.f)
	{
		bPlaying = false;
		// SetActorHiddenInGame(true);
		VisualMesh->SetHiddenInGame(true);
		TrailNiagaraComponent->Deactivate();
		if (bBroadcastWhenFinished)
		{
			OnTrailFinished.Broadcast();
		}
	}
}
