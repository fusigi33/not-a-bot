#include "FPS/Characters/Base/FPSCombatCharacter.h"

#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AFPSCombatCharacter::AFPSCombatCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFPSCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	CurrentArmor = MaxArmor;
	bDead = false;
	InvulnerableUntilTime = 0.f;
	
	if (WeaponClass)
	{
		EquippedWeapon = GetWorld()->SpawnActor<AActor>(WeaponClass);
		RestoreEquippedWeaponAttachment();
	}
}

void AFPSCombatCharacter::RestoreEquippedWeaponAttachment()
{
	if (EquippedWeapon && GetMesh())
	{
		EquippedWeapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("hand_rSocket"));
	}
}

float AFPSCombatCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (bDead || DamageAmount <= 0.f || IsInvulnerable())
	{
		return 0.f;
	}

	ApplyDamageToArmorAndHealth(DamageAmount);

	if (InvulnerabilityDuration > 0.f)
	{
		InvulnerableUntilTime = GetWorld()->GetTimeSeconds() + InvulnerabilityDuration;
	}

	K2_OnDamaged(DamageAmount, CurrentHealth, CurrentArmor, EventInstigator, DamageCauser);

	if (CurrentHealth <= 0.f)
	{
		Die(EventInstigator);
	}

	return DamageAmount;
}

bool AFPSCombatCharacter::IsInvulnerable() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < InvulnerableUntilTime;
}

void AFPSCombatCharacter::ApplyDamageToArmorAndHealth(float IncomingDamage)
{
	float RemainingDamage = IncomingDamage;

	if (CurrentArmor > 0.f)
	{
		const float ArmorDamage = FMath::Min(CurrentArmor, IncomingDamage * ArmorAbsorbRatio);
		CurrentArmor -= ArmorDamage;
		RemainingDamage -= ArmorDamage;
	}

	if (RemainingDamage > 0.f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - RemainingDamage, 0.f, MaxHealth);
	}
}

void AFPSCombatCharacter::Die(AController* KillerController)
{
	if (bDead)
	{
		return;
	}

	bDead = true;
	OnDeath();
	OnCharacterDied.Broadcast(this, KillerController);
}

void AFPSCombatCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 간단한 ragdoll 처리
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
	}

	SetLifeSpan(10.f);
}
