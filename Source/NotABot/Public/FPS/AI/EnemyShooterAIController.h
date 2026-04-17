#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyShooterAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class NOTABOT_API AEnemyShooterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyShooterAIController();

public:
	UFUNCTION(BlueprintCallable, Category="AI")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	UAIPerceptionComponent* AIPerception;
	
	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
	TWeakObjectPtr<AActor> CurrentTarget;
};