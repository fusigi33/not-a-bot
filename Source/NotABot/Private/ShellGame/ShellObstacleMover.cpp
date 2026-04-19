#include "ShellGame/ShellObstacleMover.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AShellObstacleMover::AShellObstacleMover()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(RootScene);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	PathSpline->SetupAttachment(RootScene);
}

void AShellObstacleMover::BeginPlay()
{
	Super::BeginPlay();

	SetObstacleVisible(false);
	
	if (PathSpline)
	{
		PathSpline->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	CachedPointArrayLength = GetPointArrayPathLength();
}

void AShellObstacleMover::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bMovementActive)
	{
		UpdateMovement(DeltaSeconds);
	}
}

void AShellObstacleMover::ActivateObstacleMovement()
{
	GetWorldTimerManager().ClearTimer(ActivationTimerHandle);

	if (MovementSettings.ActivationDelay <= KINDA_SMALL_NUMBER)
	{
		StartMovementNow();
		return;
	}

	GetWorldTimerManager().SetTimer(ActivationTimerHandle, this, &AShellObstacleMover::StartMovementNow, MovementSettings.ActivationDelay, false);
}

void AShellObstacleMover::DeactivateObstacleMovement()
{
	GetWorldTimerManager().ClearTimer(ActivationTimerHandle);
	bMovementActive = false;
	TravelDistance = 0.0f;
	SetObstacleVisible(false);
}

float AShellObstacleMover::GetPointArrayPathLength() const
{
	float TotalLength = 0.0f;
	for (int32 Index = 1; Index < MovementSettings.PointPath.Num(); ++Index)
	{
		TotalLength += FVector::Dist(MovementSettings.PointPath[Index - 1].WorldPoint, MovementSettings.PointPath[Index].WorldPoint);
	}
	return TotalLength;
}

FVector AShellObstacleMover::GetPointArrayLocationAtDistance(float Distance) const
{
	if (MovementSettings.PointPath.Num() == 0)
	{
		return GetActorLocation();
	}

	if (MovementSettings.PointPath.Num() == 1)
	{
		return MovementSettings.PointPath[0].WorldPoint;
	}

	float RemainingDistance = Distance;
	for (int32 Index = 1; Index < MovementSettings.PointPath.Num(); ++Index)
	{
		const FVector Start = MovementSettings.PointPath[Index - 1].WorldPoint;
		const FVector End = MovementSettings.PointPath[Index].WorldPoint;
		const float SegmentLength = FVector::Dist(Start, End);
		if (RemainingDistance <= SegmentLength)
		{
			const float Alpha = SegmentLength <= KINDA_SMALL_NUMBER ? 1.0f : RemainingDistance / SegmentLength;
			return FMath::Lerp(Start, End, Alpha);
		}

		RemainingDistance -= SegmentLength;
	}

	return MovementSettings.PointPath.Last().WorldPoint;
}

FVector AShellObstacleMover::GetLocationAtDistance(float Distance) const
{
	return (MovementSettings.PathMode == EShellObstaclePathMode::Spline)
		? PathSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World)
		: GetPointArrayLocationAtDistance(Distance);
}

FVector AShellObstacleMover::GetMovementDirectionAtDistance(float Distance) const
{
	if (MovementSettings.PathMode == EShellObstaclePathMode::Spline)
	{
		return PathSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	}

	const float PathLength = CachedPointArrayLength;
	if (PathLength <= KINDA_SMALL_NUMBER)
	{
		return GetActorForwardVector();
	}

	const float SampleDistance = FMath::Clamp(PathLength * 0.01f, 1.0f, 25.0f);
	const float NextDistance = FMath::Min(Distance + SampleDistance, PathLength);
	const float PreviousDistance = FMath::Max(Distance - SampleDistance, 0.0f);
	const FVector PreviousLocation = GetPointArrayLocationAtDistance(PreviousDistance);
	const FVector NextLocation = GetPointArrayLocationAtDistance(NextDistance);
	const FVector Direction = (NextLocation - PreviousLocation).GetSafeNormal();

	return Direction.IsNearlyZero() ? GetActorForwardVector() : Direction;
}

void AShellObstacleMover::SetObstacleVisible(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(bVisible);

	if (ObstacleMesh)
	{
		ObstacleMesh->SetHiddenInGame(!bVisible);
		ObstacleMesh->SetVisibility(bVisible, true);
	}
}

void AShellObstacleMover::UpdateMovement(float DeltaSeconds)
{
	const float PathLength = (MovementSettings.PathMode == EShellObstaclePathMode::Spline)
		? PathSpline->GetSplineLength()
		: CachedPointArrayLength;

	if (PathLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	TravelDistance += MovementSettings.Speed * DeltaSeconds;
	if (TravelDistance > PathLength)
	{
		if (MovementSettings.bLoop)
		{
			TravelDistance = FMath::Fmod(TravelDistance, PathLength);
		}
		else
		{
			TravelDistance = PathLength;
			bMovementActive = false;
			SetObstacleVisible(false);
		}
	}

	const FVector NewLocation = GetLocationAtDistance(TravelDistance);
	const FVector MovementDirection = GetMovementDirectionAtDistance(TravelDistance);

	SetActorLocation(NewLocation);
	if (!MovementDirection.IsNearlyZero())
	{
		SetActorRotation(MovementDirection.Rotation());
	}
}

void AShellObstacleMover::StartMovementNow()
{
	TravelDistance = 0.0f;
	bMovementActive = true;
	SetObstacleVisible(true);
	SetActorLocation(GetLocationAtDistance(TravelDistance));

	const FVector MovementDirection = GetMovementDirectionAtDistance(TravelDistance);
	if (!MovementDirection.IsNearlyZero())
	{
		SetActorRotation(MovementDirection.Rotation());
	}
}
