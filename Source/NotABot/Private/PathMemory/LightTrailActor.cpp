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
	bPlaying = (Board != nullptr);

	SetActorLocation(Board->GetSplineStartLocation());
	
	VisualMesh->SetHiddenInGame(false); // 구체 보이기

	// 나이아가라 시스템 에셋이 설정되어 있다면 활성화
	if (TrailSystemAsset)
	{
		TrailNiagaraComponent->SetAsset(TrailSystemAsset);
		TrailNiagaraComponent->Activate(true); // Reset 활성화
	}
}

void ALightTrailActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPlaying || !Board)
	{
		return;
	}

	// 알파 계산
	ElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedTime / MoveDuration, 0.f, 1.f);

	// 위치 업데이트
	const float Dist = Alpha * Board->GetSplineLength();
	const FVector NewLoc = Board->GetLocationAtDistance(Dist);
	SetActorLocation(NewLoc);

	// 목적지 도착했을 때
	if (Alpha >= 1.f)
	{
		bPlaying = false;
		// SetActorHiddenInGame(true);
		VisualMesh->SetHiddenInGame(true);
		TrailNiagaraComponent->Deactivate();
		OnTrailFinished.Broadcast();
	}
}