#include "DominoMiniGame/DominoRollingBallActor.h"

#include "DominoMiniGame/DominoBlockActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADominoRollingBallActor::ADominoRollingBallActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	SetRootComponent(BallMesh);

	BallMesh->SetMobility(EComponentMobility::Movable);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BallMesh->SetCollisionObjectType(ECC_PhysicsBody);
	BallMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BallMesh->SetNotifyRigidBodyCollision(true);
	BallMesh->SetUseCCD(true);
	BallMesh->SetSimulatePhysics(false);
	BallMesh->SetEnableGravity(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMeshFinder.Object);
	}
}

void ADominoRollingBallActor::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
	BallMesh->OnComponentHit.AddDynamic(this, &ADominoRollingBallActor::HandleBallHit);
	ResetDominoSimulationObject_Implementation();
}

void ADominoRollingBallActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyBallSettings();
}

void ADominoRollingBallActor::SetDominoSimulationEnabled_Implementation(bool bEnabled)
{
	bSimulationEnabled = bEnabled;
	SetBallPhysicsEnabled(bEnabled);

	if (bEnabled)
	{
		if (bWakeOnSimulationStart)
		{
			BallMesh->WakeAllRigidBodies();
		}
		else
		{
			BallMesh->PutAllRigidBodiesToSleep();
		}
	}
}

void ADominoRollingBallActor::ResetDominoSimulationObject_Implementation()
{
	bSimulationEnabled = false;
	LastHitImpulseTime = -FLT_MAX;
	SetBallPhysicsEnabled(false);
	SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

void ADominoRollingBallActor::SetBallPhysicsEnabled(bool bEnabled)
{
	if (!BallMesh)
	{
		return;
	}

	ApplyBallSettings();

	BallMesh->SetSimulatePhysics(bEnabled);
	BallMesh->SetEnableGravity(bEnabled);
	BallMesh->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::QueryOnly);

	if (!bEnabled)
	{
		BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void ADominoRollingBallActor::ApplyDominoHitImpulse(const FVector& HitLocation, const FVector& IncomingVelocity)
{
	if (!BallMesh || !BallMesh->IsSimulatingPhysics())
	{
		return;
	}

	FVector RollDirection = GetActorLocation() - HitLocation;
	RollDirection.Z = 0.0f;

	if (RollDirection.IsNearlyZero())
	{
		RollDirection = IncomingVelocity;
		RollDirection.Z = 0.0f;
	}

	if (RollDirection.IsNearlyZero())
	{
		RollDirection = GetActorForwardVector();
	}

	const FVector Impulse = RollDirection.GetSafeNormal() * DominoHitImpulseStrength + FVector::UpVector * UpwardHitImpulseStrength;
	BallMesh->WakeAllRigidBodies();
	BallMesh->AddImpulseAtLocation(Impulse, HitLocation);
}

void ADominoRollingBallActor::HandleBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bSimulationEnabled || !OtherComp || !ShouldReactToHit(OtherActor, OtherComp))
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastHitImpulseTime < HitImpulseCooldown)
	{
		return;
	}

	FVector ImpactPoint = Hit.ImpactPoint;
	if (ImpactPoint.IsNearlyZero())
	{
		ImpactPoint = BallMesh ? BallMesh->GetComponentLocation() : GetActorLocation();
	}

	const FVector IncomingVelocity = OtherComp->GetPhysicsLinearVelocityAtPoint(ImpactPoint);
	if (IncomingVelocity.Size() < MinimumDominoHitSpeed)
	{
		return;
	}

	LastHitImpulseTime = CurrentTime;
	ApplyDominoHitImpulse(ImpactPoint, IncomingVelocity);
}

void ADominoRollingBallActor::ApplyBallSettings() const
{
	if (!BallMesh)
	{
		return;
	}

	const float MeshRadius = 50.0f;
	const float RadiusScale = FMath::Max(1.0f, BallRadius) / MeshRadius;
	BallMesh->SetRelativeScale3D(FVector(RadiusScale));
	BallMesh->SetLinearDamping(LinearDamping);
	BallMesh->SetAngularDamping(AngularDamping);

	if (BallMassKg > 0.0f)
	{
		BallMesh->SetMassOverrideInKg(NAME_None, BallMassKg, true);
	}
	else
	{
		BallMesh->SetMassOverrideInKg(NAME_None, 1.0f, false);
	}
}

bool ADominoRollingBallActor::ShouldReactToHit(AActor* OtherActor, UPrimitiveComponent* OtherComp) const
{
	if (!bOnlyReactToDominoHits)
	{
		return true;
	}

	if (Cast<ADominoBlockActor>(OtherActor))
	{
		return true;
	}

	return OtherComp && Cast<ADominoBlockActor>(OtherComp->GetOwner());
}
