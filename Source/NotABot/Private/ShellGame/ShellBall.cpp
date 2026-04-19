#include "ShellGame/ShellBall.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShellGame/ShellCup.h"

AShellBall::AShellBall()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootScene);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BallMesh->SetGenerateOverlapEvents(false);
}

void AShellBall::BeginPlay()
{
	Super::BeginPlay();
	SetBallVisible(false);
}

void AShellBall::AttachToCup(AShellCup* InCup, const FVector& InLocalOffset)
{
	OwningCup = InCup;
	LocalOffset = InLocalOffset;

	if (!OwningCup)
	{
		DetachFromCup();
		return;
	}

	AttachToActor(OwningCup, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(LocalOffset);
}

void AShellBall::DetachFromCup()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	OwningCup = nullptr;
}

void AShellBall::SetBallVisible(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(false);

	if (BallMesh)
	{
		BallMesh->SetHiddenInGame(!bVisible);
	}
}
