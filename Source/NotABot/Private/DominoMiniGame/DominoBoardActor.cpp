#include "DominoMiniGame/DominoBoardActor.h"

ADominoBoardActor::ADominoBoardActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ADominoBoardActor::IsInsideBoard(const FVector& WorldLocation) const
{
	const FVector Local = GetActorTransform().InverseTransformPosition(WorldLocation);
	const FVector2D HalfSize = BoardSize * 0.5f;

	return FMath::Abs(Local.X) <= HalfSize.X && FMath::Abs(Local.Z) <= HalfSize.Y;
}

FVector ADominoBoardActor::SnapLocationToBoard(const FVector& WorldLocation) const
{
	FVector Local = GetActorTransform().InverseTransformPosition(WorldLocation);

	if (bUsePlacementSnap && SnapSize > KINDA_SMALL_NUMBER)
	{
		Local.X = FMath::GridSnap(Local.X, SnapSize);
		Local.Z = FMath::GridSnap(Local.Z, SnapSize);
	}

	Local.Y = 0.0f;

	if (bConstrainPlacementToBoardBounds)
	{
		const FVector2D HalfSize = BoardSize * 0.5f;
		Local.X = FMath::Clamp(Local.X, -HalfSize.X, HalfSize.X);
		Local.Z = FMath::Clamp(Local.Z, -HalfSize.Y, HalfSize.Y);
	}

	return GetActorTransform().TransformPosition(Local);
}

FBox ADominoBoardActor::GetBoardBounds() const
{
	const FVector Origin = GetActorLocation();
	const FVector Extent(BoardSize.X * 0.5f, BoardSize.Y * 0.5f, BoundsHeight * 0.5f);
	return FBox(Origin - Extent, Origin + Extent);
}
