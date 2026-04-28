#include "DominoMiniGame/DominoBlockActor.h"

#include "Components/StaticMeshComponent.h"

ADominoBlockActor::ADominoBlockActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetEnableGravity(false);
}

void ADominoBlockActor::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
	SetPlacementMode(true);
}

void ADominoBlockActor::SetPlacementMode(bool bInPlacementMode)
{
	if (bInPlacementMode)
	{
		SetPhysicsEnabled(false);
		MeshComponent->SetCollisionEnabled(bIsPreview ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);
	}
	else
	{
		SetPhysicsEnabled(true);
	}
}

void ADominoBlockActor::SetPhysicsEnabled(bool bEnabled)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetSimulatePhysics(bEnabled);
	MeshComponent->SetEnableGravity(bEnabled);
	MeshComponent->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::QueryOnly);

	if (!bEnabled)
	{
		MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void ADominoBlockActor::ApplyInitialFallImpulse(EDominoFallDirection Direction)
{
	if (!MeshComponent || !MeshComponent->IsSimulatingPhysics())
	{
		return;
	}

	const float DirectionSign = Direction == EDominoFallDirection::Forward ? 1.0f : -1.0f;
	const FVector AngularImpulse = GetActorRightVector() * DirectionSign * InitialAngularImpulseStrength;

	MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	MeshComponent->AddAngularImpulseInRadians(AngularImpulse, NAME_None, true);
}

bool ADominoBlockActor::IsFallen() const
{
	const float UpDot = FVector::DotProduct(GetActorUpVector(), FVector::UpVector);
	return UpDot <= FallenDotThreshold;
}

void ADominoBlockActor::SetPreviewValid(bool bIsValid)
{
	if (!bIsPreview)
	{
		return;
	}

	ApplyMaterial(bIsValid ? PreviewValidMaterial : PreviewInvalidMaterial);
}

void ADominoBlockActor::SetAsPreview(bool bPreview)
{
	bIsPreview = bPreview;

	if (bIsPreview)
	{
		SetPhysicsEnabled(false);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorEnableCollision(false);
		ApplyMaterial(PreviewValidMaterial);
	}
	else
	{
		SetActorEnableCollision(true);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ApplyMaterial(NormalMaterial);
	}
}

void ADominoBlockActor::ResetDominoTransform()
{
	SetPhysicsEnabled(false);
	SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

void ADominoBlockActor::ApplyMaterial(UMaterialInterface* Material)
{
	if (MeshComponent && Material)
	{
		MeshComponent->SetMaterial(0, Material);
	}
}
