#pragma once

#include "CoreMinimal.h"
#include "SlideMazeTypes.generated.h"

UENUM(BlueprintType)
enum class ESlideMazeDirection : uint8
{
	Up UMETA(DisplayName="Up"),
	Down UMETA(DisplayName="Down"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right")
};

UENUM(BlueprintType)
enum class ESlideMazeTileType : uint8
{
	Empty UMETA(DisplayName="Empty"),
	Wall UMETA(DisplayName="Wall"),
	Breakable UMETA(DisplayName="Breakable"),
	Start UMETA(DisplayName="Start"),
	Goal UMETA(DisplayName="Goal"),
	TeleportIn UMETA(DisplayName="Teleport In"),
	TeleportOut UMETA(DisplayName="Teleport Out")
};

USTRUCT(BlueprintType)
struct FSlideMazeMoveResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bCanMove = false;

	UPROPERTY(BlueprintReadOnly)
	bool bHitBreakable = false;

	UPROPERTY(BlueprintReadOnly)
	bool bHitTeleport = false;

	UPROPERTY(BlueprintReadOnly)
	bool bReachedGoal = false;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint StartGrid = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint StopGrid = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint BreakableGrid = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(BlueprintReadOnly)
	FIntPoint TeleportEntranceGrid = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(BlueprintReadOnly)
	FIntPoint TeleportExitGrid = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(BlueprintReadOnly)
	FString TeleportSymbolString;

	TCHAR TeleportSymbol = TEXT('\0');
};
