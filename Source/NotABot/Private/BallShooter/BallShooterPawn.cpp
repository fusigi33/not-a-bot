#include "BallShooter/BallShooterPawn.h"

#include "BallShooter/BallShooterGameManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABallShooterPawn::ABallShooterPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// The pawn itself is the projectile, so collision drives both aiming and flight.
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(16.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionObjectType(ECC_PhysicsBody);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->SetUseCCD(true);
	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(CollisionSphere);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LaunchOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("LaunchOrigin"));
	LaunchOrigin->SetupAttachment(CollisionSphere);

	// Trajectory instances render a lightweight projectile preview in-world.
	TrajectoryInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TrajectoryInstances"));
	TrajectoryInstances->SetupAttachment(CollisionSphere);
	TrajectoryInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrajectoryInstances->SetCastShadow(false);
	TrajectoryInstances->bCastDynamicShadow = false;
	TrajectoryInstances->bCastStaticShadow = false;
	TrajectoryInstances->bAffectDistanceFieldLighting = false;
	TrajectoryInstances->bCastContactShadow = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	// ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = BounceBounciness;
	ProjectileMovement->Friction = BounceFriction;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = BounceVelocityStopThreshold;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetPlaneConstraintEnabled(true);
	ProjectileMovement->SetPlaneConstraintNormal(FVector::UpVector);
	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ABallShooterPawn::HandleProjectileBounce);
}

void ABallShooterPawn::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BallShooterPawn] BeginPlay Pawn=%s ActorCollision=%s CollisionSphere=%s CollisionEnabled=%d QueryEnabled=%s GenerateOverlap=%s ObjectType=%d ResponseToPawn=%d ResponseToPhysicsBody=%d ResponseToVisibility=%d"),
		*GetName(),
		GetActorEnableCollision() ? TEXT("true") : TEXT("false"),
		CollisionSphere ? *CollisionSphere->GetName() : TEXT("None"),
		CollisionSphere ? static_cast<int32>(CollisionSphere->GetCollisionEnabled()) : -1,
		CollisionSphere && CollisionSphere->IsQueryCollisionEnabled() ? TEXT("true") : TEXT("false"),
		CollisionSphere && CollisionSphere->GetGenerateOverlapEvents() ? TEXT("true") : TEXT("false"),
		CollisionSphere ? static_cast<int32>(CollisionSphere->GetCollisionObjectType()) : -1,
		CollisionSphere ? static_cast<int32>(CollisionSphere->GetCollisionResponseToChannel(ECC_Pawn)) : -1,
		CollisionSphere ? static_cast<int32>(CollisionSphere->GetCollisionResponseToChannel(ECC_PhysicsBody)) : -1,
		CollisionSphere ? static_cast<int32>(CollisionSphere->GetCollisionResponseToChannel(ECC_Visibility)) : -1);

	// Start every round from the configured minimum launch speed.
	CurrentLaunchSpeed = MinLaunchSpeed;
	LaunchOrigin->SetRelativeRotation(FRotator(LaunchPitchDegrees, AimYawDegrees, 0.0f));
	ProjectileMovement->SnapUpdatedComponentToPlane();

	RegisterInputMappingContext();

	StopBall();
	RefreshTrajectory();
}

void ABallShooterPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	RegisterInputMappingContext();
}

void ABallShooterPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Aim and charge are mutually exclusive runtime states.
	if (AimState == EBallShooterAimState::Aiming)
	{
		UpdateAim(DeltaSeconds);
	}
	else if (AimState == EBallShooterAimState::Charging)
	{
		UpdateCharge(DeltaSeconds);
	}

	if (!bLaunched || bFinished)
	{
		return;
	}

	FlightTime += DeltaSeconds;

	// Fail the projectile if it stalls, times out, exceeds bounce limits, or leaves the board.
	const float CurrentSpeed = GetCurrentVelocity().Size();
	TimeBelowStopSpeed = CurrentSpeed <= StopSpeedThreshold ? TimeBelowStopSpeed + DeltaSeconds : 0.0f;

	if (TimeBelowStopSpeed >= TimeBelowStopSpeedToFail || FlightTime >= MaxFlightTime || BounceCount > MaxBounceCount || IsOutsideBoardBounds())
	{
		FinishBall(EBallShooterRoundResult::Unreachable);
	}
}

void ABallShooterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	// Bind analog aim input for continuous yaw updates.
	if (AimInputAction)
	{
		EnhancedInput->BindAction(AimInputAction, ETriggerEvent::Triggered, this, &ABallShooterPawn::HandleAimInput);
		EnhancedInput->BindAction(AimInputAction, ETriggerEvent::Completed, this, &ABallShooterPawn::HandleAimInputCompleted);
	}

	// Bind charge start/release so the same action controls power selection and firing.
	if (ChargeInputAction)
	{
		EnhancedInput->BindAction(ChargeInputAction, ETriggerEvent::Started, this, &ABallShooterPawn::HandleChargeStarted);
		EnhancedInput->BindAction(ChargeInputAction, ETriggerEvent::Completed, this, &ABallShooterPawn::HandleChargeCompleted);
		EnhancedInput->BindAction(ChargeInputAction, ETriggerEvent::Canceled, this, &ABallShooterPawn::HandleChargeCompleted);
	}
}

void ABallShooterPawn::SetGameManager(ABallShooterGameManager* InGameManager)
{
	GameManager = InGameManager;
}

void ABallShooterPawn::ResetAiming(const FTransform& SpawnTransform)
{
	// Align the pawn with the spawn transform before opening the aiming phase.
	StopBall();
	FTransform AdjustedSpawnTransform = SpawnTransform;
	const float CollisionRadius = CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 24.0f;
	AdjustedSpawnTransform.AddToTranslation(FVector::UpVector * (CollisionRadius + 1.0f));
	SetActorTransform(AdjustedSpawnTransform);
	ChargeTime = 0.0f;
	CurrentLaunchSpeed = MinLaunchSpeed;
	SetAimState(EBallShooterAimState::Aiming);
	LaunchOrigin->SetRelativeRotation(FRotator(LaunchPitchDegrees, AimYawDegrees, 0.0f));
	ProjectileMovement->SetPlaneConstraintOrigin(AdjustedSpawnTransform.GetLocation());
	ProjectileMovement->SnapUpdatedComponentToPlane();
	RefreshTrajectory();
}

void ABallShooterPawn::LaunchBall(const FVector& InitialVelocity)
{
	if (bLaunched || InitialVelocity.IsNearlyZero())
	{
		return;
	}

	const FVector ActorLocation = GetActorLocation();
	const FVector LaunchOriginLocation = LaunchOrigin ? LaunchOrigin->GetComponentLocation() : FVector::ZeroVector;
	const float CollisionRadius = CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 0.0f;

	TArray<AActor*> AttachedOverlaps;
	if (CollisionSphere)
	{
		CollisionSphere->GetOverlappingActors(AttachedOverlaps);
	}

	TArray<FString> AttachedOverlapNames;
	for (AActor* OverlapActor : AttachedOverlaps)
	{
		if (IsValid(OverlapActor) && OverlapActor != this)
		{
			AttachedOverlapNames.Add(FString::Printf(TEXT("%s(%s)"), *OverlapActor->GetName(), *OverlapActor->GetClass()->GetName()));
		}
	}

	TArray<FOverlapResult> WorldOverlaps;
	TArray<FString> WorldOverlapNames;
	bool bWorldOverlapDetected = false;

	if (UWorld* World = GetWorld())
	{
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BallShooterLaunchOverlap), false, this);
		if (CollisionSphere)
		{
			QueryParams.AddIgnoredComponent(CollisionSphere.Get());
		}

		bWorldOverlapDetected = World->OverlapMultiByObjectType(
			WorldOverlaps,
			ActorLocation,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(FMath::Max(CollisionRadius, 1.0f)),
			QueryParams);
	}

	for (const FOverlapResult& OverlapResult : WorldOverlaps)
	{
		const AActor* OverlapActor = OverlapResult.GetActor();
		const UPrimitiveComponent* OverlapComponent = OverlapResult.GetComponent();
		if (!IsValid(OverlapActor) || OverlapActor == this)
		{
			continue;
		}

		WorldOverlapNames.Add(
			FString::Printf(
				TEXT("%s(%s)::%s"),
				*OverlapActor->GetName(),
				*OverlapActor->GetClass()->GetName(),
				OverlapComponent ? *OverlapComponent->GetName() : TEXT("NoComponent")));
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BallShooter] LaunchBall start Pawn=%s ActorLocation=%s LaunchOrigin=%s Velocity=%s Radius=%.2f AimState=%d AttachedOverlaps=%d [%s] WorldOverlapDetected=%s WorldOverlaps=%d [%s]"),
		*GetName(),
		*ActorLocation.ToCompactString(),
		*LaunchOriginLocation.ToCompactString(),
		*InitialVelocity.ToCompactString(),
		CollisionRadius,
		static_cast<int32>(AimState),
		AttachedOverlapNames.Num(),
		AttachedOverlapNames.Num() > 0 ? *FString::Join(AttachedOverlapNames, TEXT(", ")) : TEXT("None"),
		bWorldOverlapDetected ? TEXT("true") : TEXT("false"),
		WorldOverlapNames.Num(),
		WorldOverlapNames.Num() > 0 ? *FString::Join(WorldOverlapNames, TEXT(", ")) : TEXT("None"));

	// Reset transient flight state before enabling projectile simulation.
	bLaunched = true;
	bFinished = false;
	FlightTime = 0.0f;
	TimeBelowStopSpeed = 0.0f;
	BounceCount = 0;

	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->Activate(true);
	ProjectileMovement->Velocity = InitialVelocity;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BallShooter] LaunchBall activated Pawn=%s ProjectileVelocity=%s ComponentVelocity=%s IsActive=%s UpdatedComponent=%s"),
		*GetName(),
		*ProjectileMovement->Velocity.ToCompactString(),
		CollisionSphere ? *CollisionSphere->GetComponentVelocity().ToCompactString() : TEXT("None"),
		ProjectileMovement->IsActive() ? TEXT("true") : TEXT("false"),
		ProjectileMovement->UpdatedComponent ? *ProjectileMovement->UpdatedComponent->GetName() : TEXT("None"));
}

void ABallShooterPawn::StopBall()
{
	// Clear all runtime state so the pawn can be reused for the next round.
	bLaunched = false;
	bFinished = false;
	FlightTime = 0.0f;
	TimeBelowStopSpeed = 0.0f;
	BounceCount = 0;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
}

void ABallShooterPawn::NotifyReachedGoal()
{
	if (bFinished)
	{
		return;
	}

	FinishBall(EBallShooterRoundResult::Success);
}

void ABallShooterPawn::SetBoardBounds(const FVector& InMinBounds, const FVector& InMaxBounds)
{
	BoardMinBounds = InMinBounds;
	BoardMaxBounds = InMaxBounds;
}

void ABallShooterPawn::SetAimState(EBallShooterAimState NewState)
{
	AimState = NewState;
	SetTrajectoryVisible(AimState == EBallShooterAimState::Aiming || AimState == EBallShooterAimState::Charging);

	if (AimState == EBallShooterAimState::Aiming || AimState == EBallShooterAimState::Charging)
	{
		RefreshTrajectory();
	}
}

void ABallShooterPawn::RefreshTrajectory()
{
	ClearTrajectory();

	if ((AimState != EBallShooterAimState::Aiming && AimState != EBallShooterAimState::Charging) || !GetWorld() || !TrajectoryInstances)
	{
		return;
	}

	if (!TrajectoryInstances->GetStaticMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("BallShooterPawn '%s' cannot render trajectory preview because TrajectoryInstances has no Static Mesh assigned."), *GetName());
		return;
	}

	// Simulate a preview shot and spawn one instance per sampled path point.
	FPredictProjectilePathParams Params;
	const float CollisionRadius = CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 24.0f;
	const FVector LaunchDirection = LaunchOrigin->GetForwardVector();
	Params.StartLocation = LaunchOrigin->GetComponentLocation() + (LaunchDirection * CollisionRadius) + FVector(0.0f, 0.0f, CollisionRadius + 1.0f);
	Params.LaunchVelocity = BuildLaunchVelocity(FMath::Max(CurrentLaunchSpeed, 1.0f));
	Params.OverrideGravityZ = 0.0001f;
	Params.bTraceWithCollision = true;
	Params.bTraceWithChannel = false;
	Params.ObjectTypes = BuildTrajectoryObjectTypes();
	Params.ProjectileRadius = CollisionRadius;
	Params.MaxSimTime = TrajectorySimTime;
	Params.SimFrequency = TrajectorySimFrequency;
	Params.DrawDebugType = EDrawDebugTrace::None;
	Params.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(this, Params, Result);

	if (Result.PathData.Num() <= 2)
	{
		if (!bLoggedShortTrajectoryWarning)
		{
			const AActor* HitActor = Result.HitResult.GetActor();
			const UPrimitiveComponent* HitComponent = Result.HitResult.GetComponent();
			const FString HitActorName = HitActor ? HitActor->GetName() : TEXT("None");
			const FString HitActorClass = HitActor ? HitActor->GetClass()->GetName() : TEXT("None");
			const FString HitComponentName = HitComponent ? HitComponent->GetName() : TEXT("None");
			const FVector LaunchOriginLocation = LaunchOrigin ? LaunchOrigin->GetComponentLocation() : FVector::ZeroVector;
			const FVector PawnLocation = GetActorLocation();
			const float CollisionSphereRadius = CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 0.0f;

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("BallShooterPawn '%s' predicted only %d trajectory points. PawnLocation=%s LaunchOrigin=%s Start=%s Velocity=%s PreviewRadius=%.2f CollisionSphereRadius=%.2f HitActor=%s HitActorClass=%s HitComponent=%s ImpactPoint=%s ImpactNormal=%s BlockingHit=%s"),
				*GetName(),
				Result.PathData.Num(),
				*PawnLocation.ToCompactString(),
				*LaunchOriginLocation.ToCompactString(),
				*Params.StartLocation.ToCompactString(),
				*Params.LaunchVelocity.ToCompactString(),
				Params.ProjectileRadius,
				CollisionSphereRadius,
				*HitActorName,
				*HitActorClass,
				*HitComponentName,
				*Result.HitResult.ImpactPoint.ToCompactString(),
				*Result.HitResult.ImpactNormal.ToCompactString(),
				Result.HitResult.bBlockingHit ? TEXT("true") : TEXT("false"));

			bLoggedShortTrajectoryWarning = true;
		}
	}
	else
	{
		bLoggedShortTrajectoryWarning = false;
	}

	for (int32 PointIndex = 1; PointIndex < Result.PathData.Num(); ++PointIndex)
	{
		const FPredictProjectilePathPointData& PointData = Result.PathData[PointIndex];
		const FVector TrajectoryLocation = PointData.Location + FVector(0.0f, 0.0f, TrajectoryZOffset);
		FTransform InstanceTransform(FRotator::ZeroRotator, TrajectoryLocation, FVector(TrajectoryPointScale));
		TrajectoryInstances->AddInstance(InstanceTransform, true);
	}
}

void ABallShooterPawn::HandleAimInput(const FInputActionValue& Value)
{
	if (AimState != EBallShooterAimState::Aiming)
	{
		return;
	}

	AimInputAxis = Value.Get<float>();
}

void ABallShooterPawn::HandleAimInputCompleted(const FInputActionValue& Value)
{
	AimInputAxis = 0.0f;
}

void ABallShooterPawn::HandleChargeStarted(const FInputActionValue& Value)
{
	if (AimState != EBallShooterAimState::Aiming)
	{
		return;
	}

	// Reset charge timing so each attempt starts from the minimum speed.
	ChargeTime = 0.0f;
	CurrentLaunchSpeed = MinLaunchSpeed;
	SetAimState(EBallShooterAimState::Charging);
}

void ABallShooterPawn::HandleChargeCompleted(const FInputActionValue& Value)
{
	if (AimState != EBallShooterAimState::Charging)
	{
		return;
	}

	// Once charge is released, lock aim and hand the launch velocity to the ball.
	SetAimState(EBallShooterAimState::Launched);
	LaunchBall(BuildLaunchVelocity(CurrentLaunchSpeed));
}

void ABallShooterPawn::UpdateAim(float DeltaSeconds)
{
	if (FMath::IsNearlyZero(AimInputAxis))
	{
		return;
	}

	// Clamp yaw so the player stays within the board's intended firing arc.
	AimYawDegrees = FMath::Clamp(AimYawDegrees + (AimInputAxis * AimYawSpeedDegrees * DeltaSeconds), MinAimYawDegrees, MaxAimYawDegrees);
	LaunchOrigin->SetRelativeRotation(FRotator(LaunchPitchDegrees, AimYawDegrees, 0.0f));
	RefreshTrajectory();
}

void ABallShooterPawn::UpdateCharge(float DeltaSeconds)
{
	ChargeTime += DeltaSeconds;

	// Ping-pong mode makes charge power oscillate instead of stopping at the max.
	float Alpha = FMath::Clamp(ChargeTime / ChargeDuration, 0.0f, 1.0f);
	if (bPingPongCharge)
	{
		Alpha = (FMath::Sin((ChargeTime / ChargeDuration) * PI - HALF_PI) + 1.0f) * 0.5f;
	}

	CurrentLaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, Alpha);
	RefreshTrajectory();
}

FVector ABallShooterPawn::GetCurrentVelocity() const
{
	return ProjectileMovement ? ProjectileMovement->Velocity : FVector::ZeroVector;
}

void ABallShooterPawn::HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	BounceCount++;
}

void ABallShooterPawn::SetTrajectoryVisible(bool bVisible)
{
	if (TrajectoryInstances)
	{
		TrajectoryInstances->SetVisibility(bVisible, true);
		if (!bVisible)
		{
			// Hide and clear preview points together so stale paths never remain on screen.
			ClearTrajectory();
		}
	}
}

FVector ABallShooterPawn::BuildLaunchVelocity(float Speed) const
{
	const FVector FlatDirection = FVector::VectorPlaneProject(LaunchOrigin->GetForwardVector(), FVector::UpVector).GetSafeNormal();
	return FlatDirection * Speed;
}

TArray<TEnumAsByte<EObjectTypeQuery>> ABallShooterPawn::BuildTrajectoryObjectTypes() const
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	if (!CollisionSphere)
	{
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(TrajectoryCollisionChannel));
		return ObjectTypes;
	}

	auto AddIfBlocked = [this, &ObjectTypes](ECollisionChannel Channel)
	{
		if (CollisionSphere->GetCollisionResponseToChannel(Channel) == ECR_Block)
		{
			ObjectTypes.AddUnique(UEngineTypes::ConvertToObjectType(Channel));
		}
	};

	AddIfBlocked(ECC_WorldStatic);
	AddIfBlocked(ECC_WorldDynamic);
	AddIfBlocked(ECC_PhysicsBody);
	AddIfBlocked(ECC_Vehicle);
	AddIfBlocked(ECC_Destructible);

	if (ObjectTypes.Num() == 0)
	{
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(TrajectoryCollisionChannel));
	}

	return ObjectTypes;
}

void ABallShooterPawn::ClearTrajectory()
{
	if (TrajectoryInstances)
	{
		TrajectoryInstances->ClearInstances();
	}
}

void ABallShooterPawn::RegisterInputMappingContext() const
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputMappingContext)
				{
					// Register the mapping context only for the owning local player.
					Subsystem->AddMappingContext(InputMappingContext, MappingPriority);
				}
			}
		}
	}
}

void ABallShooterPawn::FinishBall(EBallShooterRoundResult Result)
{
	if (bFinished)
	{
		return;
	}

	// Freeze the projectile before broadcasting result events.
	bFinished = true;
	bLaunched = false;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (Result == EBallShooterRoundResult::Success)
	{
		OnBallReachedGoal.Broadcast(this);
		BP_OnBallReachedGoal();
	}
	else
	{
		BP_OnBallBecameUnreachable(Result);
	}

	OnBallFinished.Broadcast(this, Result);

	if (GameManager)
	{
		GameManager->HandleBallFinished(this, Result);
	}
}

bool ABallShooterPawn::IsOutsideBoardBounds() const
{
	const FVector Location = GetActorLocation();
	return Location.X < BoardMinBounds.X || Location.Y < BoardMinBounds.Y || Location.Z < BoardMinBounds.Z
		|| Location.X > BoardMaxBounds.X || Location.Y > BoardMaxBounds.Y || Location.Z > BoardMaxBounds.Z;
}
