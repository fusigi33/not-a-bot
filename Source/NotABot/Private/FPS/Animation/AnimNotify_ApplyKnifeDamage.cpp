#include "FPS/Animation/AnimNotify_ApplyKnifeDamage.h"

#include "FPS/Characters/Enemy/EnemyShooterCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_ApplyKnifeDamage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}

	if (AEnemyShooterCharacter* Enemy = Cast<AEnemyShooterCharacter>(MeshComp->GetOwner()))
	{
		Enemy->ApplyKnifeDamageFromAnimNotify();
	}
}
