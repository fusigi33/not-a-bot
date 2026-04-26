#include "Breakout/BreakoutBrick.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ABreakoutBrick::ABreakoutBrick()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootScene);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldStatic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionBox);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABreakoutBrick::HandleBallHit(ABreakoutBall* Ball)
{
	HitPoints = FMath::Max(0, HitPoints - 1);

	if (HitPoints <= 0)
	{
		Destroy();
	}
}
