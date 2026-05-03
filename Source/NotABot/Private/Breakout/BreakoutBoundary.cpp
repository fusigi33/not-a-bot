#include "Breakout/BreakoutBoundary.h"

#include "Breakout/BreakoutBall.h"
#include "Breakout/BreakoutBrick.h"
#include "Breakout/BreakoutGameManager.h"
#include "Components/BoxComponent.h"

ABreakoutBoundary::ABreakoutBoundary()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldStatic);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABreakoutBoundary::OnBoundaryBeginOverlap);

	RefreshCollisionMode();
}

void ABreakoutBoundary::BeginPlay()
{
	Super::BeginPlay();
	RefreshCollisionMode();
}

void ABreakoutBoundary::OnBoundaryBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (BoundaryType != EBreakoutBoundaryType::KillZone)
	{
		return;
	}

	if (ABreakoutBall* Ball = Cast<ABreakoutBall>(OtherActor))
	{
		Ball->HandleKillZoneOverlap();
		return;
	}

	if (ABreakoutBrick* Brick = Cast<ABreakoutBrick>(OtherActor))
	{
		if (ABreakoutGameManager* GameManager = Cast<ABreakoutGameManager>(Brick->GetOwner()))
		{
			GameManager->HandleBrickReachedKillZone(Brick);
		}
	}
}

void ABreakoutBoundary::RefreshCollisionMode()
{
	if (!CollisionBox)
	{
		return;
	}

	if (BoundaryType == EBreakoutBoundaryType::KillZone)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetGenerateOverlapEvents(true);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	}
	else
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetGenerateOverlapEvents(false);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	}
}
