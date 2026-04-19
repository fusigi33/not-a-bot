#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellGame/ShellGameTypes.h"
#include "ShellBoard.generated.h"

class AShellBall;
class AShellCup;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShellBoardIntroFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShellBoardShuffleFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShellBoardRevealFinished, const FShellSelectionResult&, Result);

UCLASS()
class NOTABOT_API AShellBoard : public AActor
{
	GENERATED_BODY()

public:
	AShellBoard();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	bool InitializeBoard();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void ResetBoardForNewRound();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void StartIntroDrop();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void StartShuffleSequence();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	FShellSelectionResult StartRevealForSelection(AShellCup* SelectedCup);

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SetCupSelectionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="Shell Game")
	FVector GetSlotWorldLocation(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category="Shell Game")
	int32 GetSlotCount() const { return SlotLocalOffsets.Num(); }

	UFUNCTION(BlueprintPure, Category="Shell Game")
	AShellCup* GetWinningCup() const;

	UFUNCTION(BlueprintPure, Category="Shell Game")
	int32 GetWinningSlotIndex() const;

	UFUNCTION(BlueprintPure, Category="Shell Game")
	const TArray<FShellCupState>& GetCupStates() const { return CupStates; }

	UPROPERTY(BlueprintAssignable, Category="Shell Game")
	FOnShellBoardIntroFinished OnIntroFinished;

	UPROPERTY(BlueprintAssignable, Category="Shell Game")
	FOnShellBoardShuffleFinished OnShuffleFinished;

	UPROPERTY(BlueprintAssignable, Category="Shell Game")
	FOnShellBoardRevealFinished OnRevealFinished;

protected:
	void StartStepMove(const FShellShuffleStep& Step);
	void FinishCurrentShuffleStep();
	void BuildTwoCupSwapSteps();
	void BuildAllCupShuffleSteps();
	void BeginPinballChaos();
	void FinishPinballChaos();
	void UpdateIntroDrop(float DeltaSeconds);
	void UpdateStepMove(float DeltaSeconds);
	void UpdatePinballChaos(float DeltaSeconds);
	void UpdatePinballReturn(float DeltaSeconds);
	void PrepareBallAttachment();
	void ResolveWinningCupVisibility(AShellCup* CupToOpen);
	int32 FindCupStateIndex(const AShellCup* Cup) const;
	void CacheReturnTargets();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<UStaticMeshComponent> BoardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<UBoxComponent> ChaosBounds;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Shell Game")
	TArray<TObjectPtr<AShellCup>> Cups;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellBall> Ball;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Slots")
	TArray<FVector> SlotLocalOffsets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Slots")
	float IntroHoverHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Ball")
	FVector BallLocalOffset = FVector(0.0f, 0.0f, -32.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Ball")
	FVector BallStartWorldOffset = FVector(0.0f, 0.0f, -13.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Reveal")
	float RevealDuration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Reveal")
	float WrongCupRevealDelay = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shell Game|Shuffle")
	EShuffleType ShuffleType = EShuffleType::TwoCupSwap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|TwoCupSwap", meta=(EditCondition="ShuffleType == EShuffleType::TwoCupSwap"))
	int32 TwoCupSwapCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|TwoCupSwap", meta=(EditCondition="ShuffleType == EShuffleType::TwoCupSwap"))
	float TwoCupMoveDuration = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|TwoCupSwap", meta=(EditCondition="ShuffleType == EShuffleType::TwoCupSwap"))
	float TwoCupPauseDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|TwoCupSwap", meta=(EditCondition="ShuffleType == EShuffleType::TwoCupSwap"))
	float TwoCupArcHeight = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|AllCup", meta=(EditCondition="ShuffleType == EShuffleType::AllCupShuffle"))
	int32 AllCupShuffleCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|AllCup", meta=(EditCondition="ShuffleType == EShuffleType::AllCupShuffle"))
	float AllCupMoveDuration = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|AllCup", meta=(EditCondition="ShuffleType == EShuffleType::AllCupShuffle"))
	float AllCupPauseDuration = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|Pinball", meta=(EditCondition="ShuffleType == EShuffleType::PinballChaos"))
	float PinballDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|Pinball", meta=(EditCondition="ShuffleType == EShuffleType::PinballChaos"))
	float PinballSpeedMin = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|Pinball", meta=(EditCondition="ShuffleType == EShuffleType::PinballChaos"))
	float PinballSpeedMax = 950.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|Pinball", meta=(EditCondition="ShuffleType == EShuffleType::PinballChaos"))
	float PinballDamping = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Shuffle|Pinball", meta=(EditCondition="ShuffleType == EShuffleType::PinballChaos"))
	float PinballReturnDuration = 0.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TArray<FShellCupState> CupStates;

private:
	enum class EBoardAnimationMode : uint8
	{
		None,
		IntroDrop,
		ShuffleMove,
		ShufflePause,
		PinballChaos,
		PinballReturn
	};

	EBoardAnimationMode AnimationMode = EBoardAnimationMode::None;
	float AnimationElapsed = 0.0f;
	float AnimationDuration = 0.0f;
	int32 ActiveShuffleStepIndex = INDEX_NONE;
	TArray<FShellShuffleStep> ShuffleSteps;
	FShellShuffleStep ActiveStep;
	TArray<FVector> ActiveMoveStartLocations;
	TArray<FRotator> ActiveMoveStartRotations;
	TArray<FVector> ChaosVelocities;
	TArray<FVector> ChaosReturnStartLocations;
	FTimerHandle ShufflePauseTimerHandle;
	FTimerHandle RevealSecondaryTimerHandle;
	FTimerHandle RevealFinishedTimerHandle;
	FShellSelectionResult LastSelectionResult;
	FRandomStream RandomStream;
};
