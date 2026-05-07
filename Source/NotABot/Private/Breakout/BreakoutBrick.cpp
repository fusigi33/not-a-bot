#include "Breakout/BreakoutBrick.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

ABreakoutBrick::ABreakoutBrick()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldStatic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionBox);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABreakoutBrick::HandleBallHit(ABreakoutBall* Ball)
{
	HitPoints = FMath::Max(0, HitPoints - 1);

	if (HitPoints <= 0)
	{
		PlayBreakEffects();
		Destroy();
	}
}

void ABreakoutBrick::PlayBreakEffects() const
{
	if (BreakEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			BreakEffect,
			GetActorLocation(),
			GetActorRotation(),
			BreakEffectScale,
			true,
			true);
	}

	if (BreakSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, GetActorLocation());
	}
}

FVector ABreakoutBrick::GetBrickSize() const
{
	if (!CollisionBox)
	{
		return FVector::ZeroVector;
	}

	return CollisionBox->GetScaledBoxExtent() * 2.0f;
}
