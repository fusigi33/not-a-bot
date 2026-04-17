#include "FPS/Characters/Enemy/HiddenEnemyCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

AHiddenEnemyCharacter::AHiddenEnemyCharacter()
{
	bCanAttack = false;
	GetCharacterMovement()->GravityScale = 0.f;
}
