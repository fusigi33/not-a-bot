#pragma once

#include "CoreMinimal.h"
#include "EnemyShooterCharacter.h"
#include "HiddenEnemyCharacter.generated.h"

UCLASS()
class NOTABOT_API AHiddenEnemyCharacter : public AEnemyShooterCharacter
{
	GENERATED_BODY()

public:
	AHiddenEnemyCharacter();
};