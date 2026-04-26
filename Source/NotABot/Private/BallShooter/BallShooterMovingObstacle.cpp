#include "BallShooter/BallShooterMovingObstacle.h"

#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

ABallShooterMovingObstacle::ABallShooterMovingObstacle()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(SceneRoot);
	CollisionBox->SetBoxExtent(FVector(80.0f, 80.0f, 80.0f));
	ConfigureObstacleCollision(CollisionBox);
}

void ABallShooterMovingObstacle::SetMoveSpeed(float InMoveSpeed)
{
	MoveSpeed = FMath::Max(0.0f, InMoveSpeed);
}

void ABallShooterMovingObstacle::BeginPlay()
{
	Super::BeginPlay();

	CachedWorldAxis = GetActorTransform().TransformVectorNoScale(LocalMoveAxis).GetSafeNormal();
	CachedCycleLength = MoveDistance * 2.0f;
	CachedPhaseDistance = CachedCycleLength <= KINDA_SMALL_NUMBER
		? 0.0f
		: FMath::Fmod((InitialPhase / (2.0f * PI)) * CachedCycleLength, CachedCycleLength);
}

void ABallShooterMovingObstacle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MoveDistance <= KINDA_SMALL_NUMBER || MoveSpeed <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Move from the cached start location to one end point, then return to start.
	RunningTime += DeltaSeconds;
	const float DistanceAlongCycle = FMath::Fmod((RunningTime * MoveSpeed) + CachedPhaseDistance, CachedCycleLength);
	const float Offset = DistanceAlongCycle <= MoveDistance
		? DistanceAlongCycle
		: (CachedCycleLength - DistanceAlongCycle);
	SetActorLocation(StartLocation + (CachedWorldAxis * Offset));
}

void ABallShooterMovingObstacle::DrawObstacleDebug() const
{
	if (!bDrawDebug || !GetWorld() || !CollisionBox)
	{
		return;
	}

	DrawDebugBox(GetWorld(), CollisionBox->GetComponentLocation(), CollisionBox->GetScaledBoxExtent(), CollisionBox->GetComponentQuat(), DebugColor, true, -1.0f, 0, 2.0f);
}
