#include "FPS/AI/EnemyShooterAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AEnemyShooterAIController::AEnemyShooterAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2500.f;
	SightConfig->LoseSightRadius = 3000.f;
	SightConfig->PeripheralVisionAngleDegrees = 75.f;
	SightConfig->SetMaxAge(2.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyShooterAIController::OnTargetPerceptionUpdated);
}

void AEnemyShooterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		CurrentTarget = Actor;
		SetFocus(Actor);
	}
	else
	{
		if (CurrentTarget.Get() == Actor)
		{
			CurrentTarget = nullptr;
			ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}