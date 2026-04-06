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

	UPROPERTY(BlueprintAssignable)
	FOnPathTraceFinished OnPathTraceFinished;

private:
	UPROPERTY()
	TObjectPtr<APlayerController> PC;

	EPathTraceState CurrentState = EPathTraceState::Idle;

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

	void EnterPlayerTurn();
	void FinishMiniGame(bool bSuccess, float AccuracyPercent);
};
