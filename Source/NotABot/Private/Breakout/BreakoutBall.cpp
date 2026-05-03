#include "Breakout/BreakoutBall.h"

#include "Breakout/BreakoutBoundary.h"
#include "Breakout/BreakoutBrick.h"
#include "Breakout/BreakoutGameManager.h"
#include "Breakout/BreakoutPaddle.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBreakoutBall, Log, All);

ABreakoutBall::ABreakoutBall()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionSphere->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 1.0f;
	ProjectileMovement->Friction = 0.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 0.0f;
	ProjectileMovement->bAutoActivate = false;

	InitialDirection = InitialDirection.GetSafeNormal();
}

void ABreakoutBall::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();

	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ABreakoutBall::HandleProjectileBounce);
	}

	ResetBall();
}

void ABreakoutBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TimeSinceLastHit += DeltaSeconds;
	MaintainMinimumSpeed();
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

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Velocity;
		ProjectileMovement->Activate(true);
	}
}

void ABreakoutBall::ResetBall()
{
	bBallActive = false;
	LastHitActor.Reset();
	TimeSinceLastHit = BIG_NUMBER;
	Velocity = FVector::ZeroVector;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (Paddle)
	{
		const FVector ResetOffset = Paddle->GetLaunchUpDirection() * 55.0f;
		SetActorLocation(Paddle->GetActorLocation() + ResetOffset);
	}
	else
	{
		SetActorLocation(SpawnLocation);
	}
}

void ABreakoutBall::StopBall()
{
	bBallActive = false;
	Velocity = FVector::ZeroVector;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
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

FVector ABreakoutBall::GetBallVelocity() const
{
	if (ProjectileMovement)
	{
		return ProjectileMovement->Velocity;
	}

	return Velocity;
}

void ABreakoutBall::HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	UE_LOG(
		LogBreakoutBall,
		Verbose,
		TEXT("Projectile bounce: Actor=%s ImpactSpeed=%.2f CurrentProjectileSpeed=%.2f Velocity=%s"),
		*GetNameSafe(ImpactResult.GetActor()),
		ImpactVelocity.Size(),
		ProjectileMovement ? ProjectileMovement->Velocity.Size() : 0.0f,
		*ImpactVelocity.ToString());

	Velocity = ImpactVelocity;
	HandleBlockingHit(ImpactResult);
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
		Velocity = ProjectileMovement ? ProjectileMovement->Velocity : Velocity;
		ClampTravelDirection();
	}

	if (ProjectileMovement)
	{
		UE_LOG(
			LogBreakoutBall,
			Verbose,
			TEXT("Post bounce velocity: Actor=%s Speed=%.2f Velocity=%s"),
			*GetNameSafe(HitActor),
			Velocity.Size(),
			*Velocity.ToString());

		ProjectileMovement->Velocity = Velocity;
		ProjectileMovement->UpdateComponentVelocity();
	}

	LastHitActor = HitActor;
	TimeSinceLastHit = 0.0f;
}

void ABreakoutBall::HandlePaddleBounce(ABreakoutPaddle* HitPaddle, const FHitResult& Hit)
{
	const FVector NewDirection = BuildPaddleBounceDirection(HitPaddle, Hit);
	const float NewSpeed = FMath::Min(GetCurrentSpeed() + SpeedIncreasePerPaddleHit, MaxSpeed);

	Velocity = NewDirection * NewSpeed;
	ClampTravelDirection();
}

void ABreakoutBall::MaintainMinimumSpeed()
{
	if (!bBallActive || !ProjectileMovement)
	{
		return;
	}

	const FVector CurrentVelocity = ProjectileMovement->Velocity;
	const float CurrentSpeedSquared = CurrentVelocity.SizeSquared();
	constexpr float SpeedCorrectionTolerance = 0.5f;
	const float MinimumCorrectableSpeed = FMath::Max(InitialSpeed - SpeedCorrectionTolerance, 0.0f);
	if (CurrentSpeedSquared >= FMath::Square(MinimumCorrectableSpeed))
	{
		Velocity = CurrentVelocity;
		return;
	}

	FVector Direction = CurrentVelocity.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = Velocity.GetSafeNormal();
	}
	if (Direction.IsNearlyZero())
	{
		Direction = InitialDirection.GetSafeNormal();
	}

	UE_LOG(
		LogBreakoutBall,
		Warning,
		TEXT("Ball speed below InitialSpeed. CurrentSpeed=%.4f InitialSpeed=%.4f Tolerance=%.4f ProjectileVelocity=%s CachedVelocity=%s CorrectedDirection=%s"),
		FMath::Sqrt(CurrentSpeedSquared),
		InitialSpeed,
		SpeedCorrectionTolerance,
		*CurrentVelocity.ToString(),
		*Velocity.ToString(),
		*Direction.ToString());

	Velocity = Direction * InitialSpeed;
	ProjectileMovement->Velocity = Velocity;
	ProjectileMovement->UpdateComponentVelocity();
}

void ABreakoutBall::ClampTravelDirection()
{
	FVector Direction = Velocity.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = InitialDirection.GetSafeNormal();
	}

	const float HorizontalSign = FMath::IsNearlyZero(Direction.Y) ? 1.0f : FMath::Sign(Direction.Y);
	const float VerticalSign = FMath::IsNearlyZero(Direction.Z) ? FMath::Sign(InitialDirection.Z) : FMath::Sign(Direction.Z);
	float MinAngleRadians = FMath::Max(
		FMath::DegreesToRadians(MinLaunchAngleDegrees),
		FMath::Acos(FMath::Clamp(MaxVerticalRatio, 0.0f, 1.0f)));
	float MaxAngleRadians = FMath::Min(
		FMath::DegreesToRadians(MaxLaunchAngleDegrees),
		FMath::Asin(FMath::Clamp(MaxHorizontalRatio, 0.0f, 1.0f)));
	if (MinAngleRadians > MaxAngleRadians)
	{
		const float FallbackAngleRadians = (MinAngleRadians + MaxAngleRadians) * 0.5f;
		MinAngleRadians = FallbackAngleRadians;
		MaxAngleRadians = FallbackAngleRadians;
	}

	const float ClampedAngleRadians = FMath::Clamp(
		FMath::Atan2(FMath::Abs(Direction.Y), FMath::Abs(Direction.Z)),
		MinAngleRadians,
		MaxAngleRadians);

	Direction.X = 0.0f;
	Direction.Y = FMath::Sin(ClampedAngleRadians) * HorizontalSign;
	Direction.Z = FMath::Cos(ClampedAngleRadians) * VerticalSign;
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

	FVector BounceDirection(0.0f, HorizontalMagnitude, FMath::Abs(VerticalMagnitude));
	if (FMath::IsNearlyZero(BounceDirection.Y, KINDA_SMALL_NUMBER))
	{
		const float ExistingHorizontal = FMath::Sign(Velocity.Y);
		BounceDirection.Y = FMath::Sin(MinAngleRadians) * (ExistingHorizontal == 0.0f ? 1.0f : ExistingHorizontal);
		BounceDirection.Z = FMath::Cos(MinAngleRadians);
	}

	return BounceDirection.GetSafeNormal();
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
