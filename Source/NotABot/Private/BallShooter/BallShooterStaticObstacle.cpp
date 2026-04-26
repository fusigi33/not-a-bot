#include "BallShooter/BallShooterStaticObstacle.h"

#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

ABallShooterStaticObstacle::ABallShooterStaticObstacle()
{
	// Static obstacles rely entirely on placed transforms, so ticking stays disabled.
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(SceneRoot);
	CollisionBox->SetBoxExtent(FVector(80.0f, 80.0f, 80.0f));
	ConfigureObstacleCollision(CollisionBox);
}

void ABallShooterStaticObstacle::DrawObstacleDebug() const
{
	if (!bDrawDebug || !GetWorld() || !CollisionBox)
	{
		return;
	}

	DrawDebugBox(GetWorld(), CollisionBox->GetComponentLocation(), CollisionBox->GetScaledBoxExtent(), CollisionBox->GetComponentQuat(), DebugColor, true, -1.0f, 0, 2.0f);
}
