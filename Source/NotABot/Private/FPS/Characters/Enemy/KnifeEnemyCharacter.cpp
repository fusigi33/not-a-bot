#include "FPS/Characters/Enemy/KnifeEnemyCharacter.h"

AKnifeEnemyCharacter::AKnifeEnemyCharacter()
{
	EnemyWeaponType = EWeaponType::Knife;
	bCanAttack = true;
	bApplyKnifeDamageFromAnimNotify = true;
	KnifeAimTime = 0.15f;
	KnifeCooldownTime = 0.6f;
}

void AKnifeEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead || !bCanAttack)
	{
		CachedKnifeTargetDistance = 0.f;
		bCachedTargetInKnifeRange = false;
		return;
	}

	AActor* Target = RecognizedTarget.Get();
	if (!Target)
	{
		CachedKnifeTargetDistance = 0.f;
		bCachedTargetInKnifeRange = false;
		return;
	}

	CachedKnifeTargetDistance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	bCachedTargetInKnifeRange = IsTargetInKnifeRange(Target);
}
