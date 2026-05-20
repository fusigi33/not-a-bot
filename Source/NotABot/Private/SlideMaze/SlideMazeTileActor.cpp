#include "SlideMaze/SlideMazeTileActor.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASlideMazeTileActor::ASlideMazeTileActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
		MeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.05f));
	}
}

void ASlideMazeTileActor::SetTileType(ESlideMazeTileType NewType)
{
	TileType = NewType;
	UpdateVisual();
}

void ASlideMazeTileActor::SetTileSymbol(const FString& NewSymbol)
{
	TileSymbolString = NewSymbol;
	TileSymbol = NewSymbol.Len() > 0 ? NewSymbol[0] : TEXT('□');
}

void ASlideMazeTileActor::SetTileSymbol(TCHAR NewSymbol)
{
	TileSymbol = NewSymbol;
	TileSymbolString = FString::Chr(NewSymbol);
}

void ASlideMazeTileActor::SetBroken()
{
	TileType = ESlideMazeTileType::Empty;
	TileSymbol = TEXT('□');
	TileSymbolString = TEXT("□");

	if (bHideWhenBroken)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
	else
	{
		UpdateVisual();
	}
}

void ASlideMazeTileActor::UpdateVisual()
{
	UMaterialInterface* MaterialToUse = GetMaterialForTileType(TileType);

	if (MaterialToUse && MeshComponent)
	{
		MeshComponent->SetMaterial(0, MaterialToUse);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("SlideMaze tile material is not assigned for tile symbol %s."), *TileSymbolString);
	}
}

UStaticMesh* ASlideMazeTileActor::GetTileMesh() const
{
	return MeshComponent ? MeshComponent->GetStaticMesh() : nullptr;
}

FVector ASlideMazeTileActor::GetTileMeshRelativeScale() const
{
	return MeshComponent ? MeshComponent->GetRelativeScale3D() : FVector::OneVector;
}

UMaterialInterface* ASlideMazeTileActor::GetMaterialForTileType(ESlideMazeTileType Type) const
{
	switch (Type)
	{
	case ESlideMazeTileType::Empty:
		return EmptyMaterial;
	case ESlideMazeTileType::Wall:
		return WallMaterial;
	case ESlideMazeTileType::Breakable:
		return BreakableMaterial;
	case ESlideMazeTileType::Start:
		return StartMaterial;
	case ESlideMazeTileType::Goal:
		return GoalMaterial;
	case ESlideMazeTileType::TeleportIn:
		return TeleportInMaterial;
	case ESlideMazeTileType::TeleportOut:
		return TeleportOutMaterial;
	default:
		return nullptr;
	}
}
