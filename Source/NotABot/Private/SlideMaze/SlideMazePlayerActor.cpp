#include "SlideMaze/SlideMazePlayerActor.h"

#include "Components/SkeletalMeshComponent.h"

ASlideMazePlayerActor::ASlideMazePlayerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);


	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	MeshComponent->SetRelativeScale3D(FVector(0.55f));
}

void ASlideMazePlayerActor::SetGridPosition(FIntPoint NewGrid, FVector NewWorldLocation)
{
	CurrentGrid = NewGrid;
	SetActorLocation(NewWorldLocation);
}

void ASlideMazePlayerActor::SetVisualState(bool bIsActive)
{
	SetActorHiddenInGame(!bIsActive);
}
