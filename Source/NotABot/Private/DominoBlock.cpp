#include "DominoBlock.h"
#include "Components/StaticMeshComponent.h"

ADominoBlock::ADominoBlock()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void ADominoBlock::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}

void ADominoBlock::SetPhysicsEnabled(bool bEnabled)
{
	Mesh->SetSimulatePhysics(bEnabled);

	if (bEnabled)
	{
		Mesh->WakeAllRigidBodies();
	}
	else
	{
		Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void ADominoBlock::ResetDomino()
{
	SetPhysicsEnabled(false);
	SetActorTransform(InitialTransform);
}

bool ADominoBlock::IsKnockedDown(float DotThreshold) const
{
	const FVector Up = GetActorUpVector();
	const float Dot = FVector::DotProduct(Up, FVector::UpVector);
	return Dot < DotThreshold;
}