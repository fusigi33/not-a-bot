#pragma once

#include "CoreMinimal.h"
#include "ShellGameTypes.generated.h"

class AShellCup;

UENUM(BlueprintType)
enum class EShellGameState : uint8
{
	Idle,
	IntroDrop,
	Shuffling,
	AwaitSelection,
	RevealResult,
	Resetting
};

UENUM(BlueprintType)
enum class EShuffleType : uint8
{
	TwoCupSwap,
	AllCupShuffle,
	PinballChaos
};

UENUM(BlueprintType)
enum class EShellObstaclePathMode : uint8
{
	Spline,
	PointArray
};

USTRUCT(BlueprintType)
struct NOTABOT_API FShellCupState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellCup> Cup = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	int32 CurrentSlotIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	bool bContainsBall = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	bool bIsSelectable = false;
};

USTRUCT(BlueprintType)
struct NOTABOT_API FShellShuffleStep
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	EShuffleType ShuffleType = EShuffleType::TwoCupSwap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TArray<int32> CupIndices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TArray<int32> FromSlotIndices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TArray<int32> ToSlotIndices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	float MoveDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	float PauseDuration = 0.15f;
};

USTRUCT(BlueprintType)
struct NOTABOT_API FShellObstaclePathPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	FVector WorldPoint = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct NOTABOT_API FShellObstacleMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	EShellObstaclePathMode PathMode = EShellObstaclePathMode::Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	float Speed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	float ActivationDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game")
	bool bLoop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game", meta=(EditCondition="PathMode == EShellObstaclePathMode::PointArray"))
	TArray<FShellObstaclePathPoint> PointPath;
};

USTRUCT(BlueprintType)
struct NOTABOT_API FShellSelectionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	bool bValidSelection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	bool bIsCorrect = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	int32 SelectedSlotIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	int32 WinningSlotIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellCup> SelectedCup = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellCup> WinningCup = nullptr;
};
