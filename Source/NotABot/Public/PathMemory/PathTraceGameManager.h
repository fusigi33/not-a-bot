#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathTraceGameManager.generated.h"

class APathTraceBoardActor;
class ALightTrailActor;
class APathTraceCharacter;
class APlayerController;
class UUserWidget;

UENUM(BlueprintType)
enum class EPathTraceState : uint8
{
	Idle,
	ShowingAnswer,
	PlayerTurn,
	Result
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPathTraceFinished, bool, bSuccess, float, AccuracyPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordedPlayerPathFinished, bool, bSuccess);

UCLASS()
class NOTABOT_API APathTraceGameManager : public AActor
{
	GENERATED_BODY()

public:
	APathTraceGameManager();

	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace")
	TObjectPtr<APathTraceBoardActor> BoardActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace")
	TObjectPtr<ALightTrailActor> LightTrailActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace")
	TObjectPtr<APathTraceCharacter> PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace")
	float RequiredAccuracyPercent = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PathTrace")
	float PerfectAccuracyDeviation = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PathTrace")
	float AutoPossessDelaySeconds = 1.0f;

	UPROPERTY(BlueprintAssignable)
	FOnPathTraceFinished OnPathTraceFinished;

	UPROPERTY(BlueprintAssignable, Category="PathTrace|Review")
	FOnRecordedPlayerPathFinished OnRecordedPlayerPathFinished;

private:
	UPROPERTY()
	TObjectPtr<APlayerController> PC;

	EPathTraceState CurrentState = EPathTraceState::Idle;
	bool bWaitingForRecordedPlayerPath = false;
	bool bRecordedPlayerPathSuccess = false;

public:
	UFUNCTION(BlueprintCallable)
	void StartMiniGame();

	UFUNCTION()
	void HandleLightTrailFinished();

	UFUNCTION()
	void HandleGoalReached(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	float EvaluateAccuracyPercent() const;

	UFUNCTION(BlueprintCallable, Category="PathTrace|Review")
	void ShowAnswerPath(float LifeTime = -1.f) const;

	UFUNCTION(BlueprintCallable, Category="PathTrace|Review")
	void ShowRecordedPlayerPath(float Duration = -1.f);

	void EnterPlayerTurn();
	void FinishMiniGame(bool bSuccess, float AccuracyPercent);
};
