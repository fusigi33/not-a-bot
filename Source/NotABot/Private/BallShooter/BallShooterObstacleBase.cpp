#include "BallShooter/BallShooterObstacleBase.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ABallShooterObstacleBase::ABallShooterObstacleBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(SceneRoot);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABallShooterObstacleBase::BeginPlay()
{
	Super::BeginPlay();
	// Cache the placed transform so derived obstacles can animate relative to it.
	StartLocation = GetActorLocation();
	StartRotation = GetActorRotation();
	DrawObstacleDebug();
}

void ABallShooterObstacleBase::ConfigureObstacleCollision(UPrimitiveComponent* CollisionComponent) const
{
	if (!CollisionComponent)
	{
		return;
	}

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ABallShooterObstacleBase::DrawObstacleDebug() const
{
}
