#include "Breakout/BreakoutBall.h"

#include "Breakout/BreakoutBoundary.h"
#include "Breakout/BreakoutBrick.h"
#include "Breakout/BreakoutGameManager.h"
#include "Breakout/BreakoutPaddle.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

ABreakoutBall::ABreakoutBall()
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootScene);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionSphere->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InitialDirection = InitialDirection.GetSafeNormal();
}

void ABreakoutBall::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	ResetBall();
}

void ABreakoutBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TimeSinceLastHit += DeltaSeconds;

	if (bBallActive)
	{
		MoveBall(DeltaSeconds);
	}
}

void ABreakoutBall::LaunchBall()
{
	if (bBallActive)
	{
		return;
	}

	Velocity = InitialDirection.GetSafeNormal() * InitialSpeed;
	ClampTravelDirection();
	bBallActive = true;
}

void ABreakoutBall::ResetBall()
{
	bBallActive = false;
	LastHitActor.Reset();
	TimeSinceLastHit = BIG_NUMBER;
	Velocity = FVector::ZeroVector;

	if (Paddle)
	{
		const FVector ResetOffset = Paddle->GetLaunchUpDirection() * 55.0f;
		SetActorLocation(Paddle->GetActorLocation() + ResetOffset);
	}
	else
	{
		SetActorLocation(SpawnLocation);
	}

	LaunchBall();
}

void ABreakoutBall::StopBall()
{
	bBallActive = false;
	Velocity = FVector::ZeroVector;
}

void ABreakoutBall::HandleKillZoneOverlap()
{
	if (GameManager)
	{
		GameManager->HandleBallLost(this);
	}
	else
	{
		StopBall();
	}
}

void ABreakoutBall::MoveBall(float DeltaSeconds)
{
	constexpr int32 MaxBouncesPerTick = 4;
	float RemainingTime = DeltaSeconds;
	int32 Iteration = 0;

	while (RemainingTime > KINDA_SMALL_NUMBER && Iteration < MaxBouncesPerTick && bBallActive)
	{
		const FVector DeltaMove = Velocity * RemainingTime;
		FHitResult Hit;
		AddActorWorldOffset(DeltaMove, true, &Hit);

		if (!Hit.bBlockingHit)
		{
			break;
		}

		const float TimeUsed = RemainingTime * Hit.Time;
		RemainingTime -= TimeUsed;
		HandleBlockingHit(Hit);
		Iteration++;
	}
}

void ABreakoutBall::HandleBlockingHit(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !CanBounceOnActor(HitActor))
	{
		return;
	}

	if (ABreakoutBoundary* Boundary = Cast<ABreakoutBoundary>(HitActor))
	{
		if (Boundary->IsKillZone())
		{
			HandleKillZoneOverlap();
			return;
		}
	}

	if (ABreakoutBrick* Brick = Cast<ABreakoutBrick>(HitActor))
	{
		Brick->HandleBallHit(this);
	}

	if (ABreakoutPaddle* HitPaddle = Cast<ABreakoutPaddle>(HitActor))
	{
		HandlePaddleBounce(HitPaddle, Hit);
	}
	else
	{
		HandleGenericBounce(Hit);
	}

	LastHitActor = HitActor;
	TimeSinceLastHit = 0.0f;
}

void ABreakoutBall::HandlePaddleBounce(ABreakoutPaddle* HitPaddle, const FHitResult& Hit)
{
	const FVector NewDirection = BuildPaddleBounceDirection(HitPaddle, Hit);
	const float NewSpeed = FMath::Min(GetCurrentSpeed() + SpeedIncreasePerPaddleHit, MaxSpeed);

	Velocity = NewDirection * NewSpeed;
	SetActorLocation(Hit.ImpactPoint + (NewDirection * SurfaceSeparationDistance));
}

void ABreakoutBall::HandleGenericBounce(const FHitResult& Hit)
{
	const FVector ReflectedDirection = FVector::VectorPlaneProject(
		FMath::GetReflectionVector(Velocity.GetSafeNormal(), Hit.ImpactNormal.GetSafeNormal()),
		FVector::RightVector ^ FVector::UpVector).GetSafeNormal();

	Velocity = ReflectedDirection * GetCurrentSpeed();
	ClampTravelDirection();
	SetActorLocation(Hit.ImpactPoint + (Velocity.GetSafeNormal() * SurfaceSeparationDistance));
}

void ABreakoutBall::ClampTravelDirection()
{
	FVector Direction = Velocity.GetSafeNormal();

	if (Direction.Z < 0.0f && FMath::Abs(Direction.Z) < (1.0f - MaxHorizontalRatio))
	{
		Direction.Z = -FMath::Max(FMath::Abs(Direction.Z), 1.0f - MaxHorizontalRatio);
	}
	else if (Direction.Z > 0.0f && FMath::Abs(Direction.Z) < (1.0f - MaxHorizontalRatio))
	{
		Direction.Z = FMath::Max(FMath::Abs(Direction.Z), 1.0f - MaxHorizontalRatio);
	}

	if (FMath::Abs(Direction.Z) > MaxVerticalRatio)
	{
		Direction.Z = FMath::Sign(Direction.Z) * MaxVerticalRatio;
	}

	Direction.X = FMath::Clamp(Direction.X, -MaxHorizontalRatio, MaxHorizontalRatio);
	Direction.Y = 0.0f;
	Direction.Normalize();
	Velocity = Direction * FMath::Clamp(GetCurrentSpeed(), InitialSpeed, MaxSpeed);
}

FVector ABreakoutBall::BuildPaddleBounceDirection(const ABreakoutPaddle* HitPaddle, const FHitResult& Hit) const
{
	const float HitOffset = HitPaddle ? HitPaddle->GetNormalizedHitOffset(Hit.ImpactPoint) : 0.0f;
	const float MinAngleRadians = FMath::DegreesToRadians(MinLaunchAngleDegrees);
	const float MaxAngleRadians = FMath::DegreesToRadians(MaxLaunchAngleDegrees);
	const float Blend = FMath::Abs(HitOffset);
	const float LaunchAngle = FMath::Lerp(MinAngleRadians, MaxAngleRadians, Blend);
	const float HorizontalSign = FMath::Sign(HitOffset);

	const float HorizontalMagnitude = FMath::Sin(LaunchAngle) * HorizontalSign;
	const float VerticalMagnitude = FMath::Cos(LaunchAngle);

	FVector BounceDirection(HorizontalMagnitude, 0.0f, FMath::Abs(VerticalMagnitude));
	if (FMath::IsNearlyZero(BounceDirection.X, KINDA_SMALL_NUMBER))
	{
		const float ExistingHorizontal = FMath::Sign(Velocity.X);
		BounceDirection.X = FMath::Sin(MinAngleRadians) * (ExistingHorizontal == 0.0f ? 1.0f : ExistingHorizontal);
		BounceDirection.Z = FMath::Cos(MinAngleRadians);
	}

	BounceDirection.Normalize();
	return BounceDirection;
}

float ABreakoutBall::GetCurrentSpeed() const
{
	return FMath::Clamp(Velocity.Size(), InitialSpeed, MaxSpeed);
}

bool ABreakoutBall::CanBounceOnActor(const AActor* HitActor) const
{
	if (!HitActor)
	{
		return false;
	}

	if (LastHitActor.Get() == HitActor && TimeSinceLastHit < SameActorBounceCooldown)
	{
		return false;
	}

	return true;
}
