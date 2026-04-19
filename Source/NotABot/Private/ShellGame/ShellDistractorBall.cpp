#include "ShellGame/ShellDistractorBall.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AShellDistractorBall::AShellDistractorBall()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(BallRadius);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCanEverAffectNavigation(false);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(CollisionSphere);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BallMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->ProjectileGravityScale = GravityScale;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = BounceFactor;
	ProjectileMovement->Friction = Friction;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 60.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bAutoActivate = false;
}

void AShellDistractorBall::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->SetSphereRadius(BallRadius);
	BallMesh->SetRelativeScale3D(FVector(BallRadius / 50.0f));
	ProjectileMovement->ProjectileGravityScale = GravityScale;
	ProjectileMovement->Bounciness = BounceFactor;
	ProjectileMovement->Friction = Friction;

	SetLifeSpan(LifeSeconds);
}

void AShellDistractorBall::Launch(const FVector& InitialVelocity)
{
	if (!ProjectileMovement)
	{
		return;
	}

	ProjectileMovement->Velocity = InitialVelocity;
	ProjectileMovement->Activate(true);
	ProjectileMovement->UpdateComponentVelocity();
}
