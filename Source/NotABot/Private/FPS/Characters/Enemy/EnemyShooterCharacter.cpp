#include "FPS/Characters/Enemy/EnemyShooterCharacter.h"

#include "FPS/AI/EnemyShooterAIController.h"
#include "FPS/Weapons/Guns/ShotgunComponent.h"
#include "FPS/Weapons/Guns/PistolWeaponComponent.h"
#include "FPS/Weapons/Guns/RifleWeaponComponent.h"
#include "FPS/Weapons/Melee/KnifeWeaponComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AEnemyShooterCharacter::AEnemyShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyShooterAIController::StaticClass();

	ShotgunComponent = CreateDefaultSubobject<UShotgunComponent>(TEXT("ShotgunComponent"));
	PistolComponent = CreateDefaultSubobject<UPistolWeaponComponent>(TEXT("PistolComponent"));
	RifleComponent = CreateDefaultSubobject<URifleWeaponComponent>(TEXT("RifleComponent"));
	KnifeComponent = CreateDefaultSubobject<UKnifeWeaponComponent>(TEXT("KnifeComponent"));

	GetCharacterMovement()->DisableMovement();

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AEnemyShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	HandleInitialPose();
	EnterState(EEnemyAttackState::Idle);
}

void AEnemyShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead || !bCanAttack)
	{
		return;
	}

	AActor* Target = GetTargetActor();
	if (Target)
	{
		if (RecognizedTarget.Get() != Target)
		{
			RecognizedTarget = Target;
			TargetRecognitionStartTime = GetWorld()->GetTimeSeconds();
		}

		LastSeenTargetTime = GetWorld()->GetTimeSeconds();
	}
	else
	{
		RecognizedTarget = nullptr;
		TargetRecognitionStartTime = -100.f;
	}

	UpdateAttackState(DeltaSeconds);
}

void AEnemyShooterCharacter::HandleInitialPose()
{
	switch (PoseType)
	{
	case EEnemyPoseType::Standing:
		UnCrouch();
		break;

	case EEnemyPoseType::Crouching:
	case EEnemyPoseType::HidingCorner:
		Crouch();
		break;

	case EEnemyPoseType::CeilingAttached:
		GetCharacterMovement()->GravityScale = 0.f;
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		SetActorRotation(GetActorRotation() + FRotator(180.f, 0.f, 0.f));
		break;
	}
}

AActor* AEnemyShooterCharacter::GetTargetActor() const
{
	if (const AEnemyShooterAIController* AIC = Cast<AEnemyShooterAIController>(GetController()))
	{
		return AIC->GetCurrentTarget();
	}

	return nullptr;
}

FVector AEnemyShooterCharacter::GetAttackStartLocation() const
{
	return GetActorLocation() + GetActorForwardVector() * 60.f + FVector(0.f, 0.f, 50.f);
}

FVector AEnemyShooterCharacter::GetAimDirectionToTarget(AActor* Target, bool bApplySpread) const
{
	if (!Target)
	{
		return GetActorForwardVector();
	}

	const FVector Start = GetAttackStartLocation();
	const FVector TargetPoint = Target->GetActorLocation() + FVector(0.f, 0.f, 50.f);

	FVector AimDir = (TargetPoint - Start).GetSafeNormal();

	if (bApplySpread)
	{
		AimDir = FMath::VRandCone(
			AimDir,
			FMath::DegreesToRadians(AimInaccuracyDegrees),
			FMath::DegreesToRadians(AimInaccuracyDegrees)
		).GetSafeNormal();
	}

	return AimDir;
}

bool AEnemyShooterCharacter::IsTargetInKnifeRange(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	return FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(KnifeAttackRange);
}

bool AEnemyShooterCharacter::HasValidTarget() const
{
	const float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastSeenTargetTime) <= LoseTargetGraceTime;
}

void AEnemyShooterCharacter::EnterState(EEnemyAttackState NewState)
{
	AttackState = NewState;
	StateStartTime = GetWorld()->GetTimeSeconds();

	if (NewState != EEnemyAttackState::FiringBurst)
	{
		BurstShotsFired = 0;
	}
}

void AEnemyShooterCharacter::UpdateAttackState(float DeltaSeconds)
{
	AActor* Target = GetTargetActor();

	switch (AttackState)
	{
	case EEnemyAttackState::Idle:
		UpdateIdleState(Target);
		break;

	case EEnemyAttackState::Aiming:
		UpdateAimingState(Target);
		break;

	case EEnemyAttackState::FiringBurst:
		UpdateFiringBurstState(Target);
		break;

	case EEnemyAttackState::Cooldown:
		UpdateCooldownState(Target);
		break;
	}
}

void AEnemyShooterCharacter::UpdateIdleState(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	if (EnemyWeaponType == EWeaponType::Knife && !IsTargetInKnifeRange(Target))
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if ((Now - TargetRecognitionStartTime) < TargetRecognitionDelay)
	{
		return;
	}

	EnterState(EEnemyAttackState::Aiming);
}

void AEnemyShooterCharacter::UpdateAimingState(AActor* Target)
{
	if (!HasValidTarget())
	{
		EnterState(EEnemyAttackState::Idle);
		return;
	}

	if (EnemyWeaponType == EWeaponType::Knife && (!Target || !IsTargetInKnifeRange(Target)))
	{
		EnterState(EEnemyAttackState::Idle);
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - StateStartTime;
	if (Elapsed < GetCurrentAimTime())
	{
		return;
	}

	if (EnemyWeaponType == EWeaponType::Rifle)
	{
		EnterState(EEnemyAttackState::FiringBurst);
		LastBurstShotTime = -100.f;
		BurstShotsFired = 0;
	}
	else
	{
		const bool bFired = FireCurrentWeapon(Target);
		if (bFired)
		{
			EnterState(EEnemyAttackState::Cooldown);
		}
		else
		{
			EnterState(EEnemyAttackState::Idle);
		}
	}
}

void AEnemyShooterCharacter::UpdateFiringBurstState(AActor* Target)
{
	if (!HasValidTarget())
	{
		EnterState(EEnemyAttackState::Idle);
		return;
	}

	if (!Target)
	{
		EnterState(EEnemyAttackState::Idle);
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastBurstShotTime < RifleBurstInterval)
	{
		return;
	}

	const bool bFired = FireCurrentWeapon(Target);
	if (!bFired)
	{
		EnterState(EEnemyAttackState::Idle);
		return;
	}

	LastBurstShotTime = Now;
	++BurstShotsFired;

	if (BurstShotsFired >= RifleBurstCount)
	{
		EnterState(EEnemyAttackState::Cooldown);
	}
}

void AEnemyShooterCharacter::UpdateCooldownState(AActor* Target)
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - StateStartTime;
	if (Elapsed < GetCurrentCooldownTime())
	{
		return;
	}

	if (Target && HasValidTarget())
	{
		if (EnemyWeaponType == EWeaponType::Knife && !IsTargetInKnifeRange(Target))
		{
			EnterState(EEnemyAttackState::Idle);
			return;
		}

		EnterState(EEnemyAttackState::Aiming);
	}
	else
	{
		EnterState(EEnemyAttackState::Idle);
	}
}

bool AEnemyShooterCharacter::FireCurrentWeapon(AActor* Target)
{
	if (!Target)
	{
		return false;
	}

	const FVector Start = GetAttackStartLocation();

	switch (EnemyWeaponType)
	{
	case EWeaponType::Shotgun:
		if (ShotgunComponent)
		{
			const FVector AimDir = GetAimDirectionToTarget(Target, true);
			return ShotgunComponent->FireShotgun(this, Start, AimDir, 0.f);
		}
		break;

	case EWeaponType::Pistol:
		if (PistolComponent)
		{
			const FVector AimDir = GetAimDirectionToTarget(Target, true);
			return PistolComponent->TryUse(this, Start, AimDir);
		}
		break;

	case EWeaponType::Rifle:
		if (RifleComponent)
		{
			const FVector AimDir = GetAimDirectionToTarget(Target, true);
			return RifleComponent->TryUse(this, Start, AimDir);
		}
		break;

	case EWeaponType::Knife:
		if (KnifeComponent && IsTargetInKnifeRange(Target))
		{
			const FVector AimDir = GetAimDirectionToTarget(Target, false);
			if (bApplyKnifeDamageFromAnimNotify)
			{
				if (KnifeComponent->TryStartUse(this, Start))
				{
					PendingKnifeTarget = Target;
					bKnifeDamagePending = true;
					return true;
				}

				return false;
			}

			return KnifeComponent->TryUse(this, Start, AimDir);
		}
		break;
	}

	return false;
}

void AEnemyShooterCharacter::ApplyKnifeDamageFromAnimNotify()
{
	if (!bKnifeDamagePending || EnemyWeaponType != EWeaponType::Knife || !KnifeComponent)
	{
		return;
	}

	AActor* Target = PendingKnifeTarget.Get();
	if (!Target || !IsTargetInKnifeRange(Target))
	{
		bKnifeDamagePending = false;
		PendingKnifeTarget = nullptr;
		return;
	}

	const FVector Start = GetAttackStartLocation();
	const FVector AimDir = GetAimDirectionToTarget(Target, false);
	KnifeComponent->ApplyKnifeDamage(this, Start, AimDir);

	bKnifeDamagePending = false;
	PendingKnifeTarget = nullptr;
}

float AEnemyShooterCharacter::GetCurrentAimTime() const
{
	switch (EnemyWeaponType)
	{
	case EWeaponType::Shotgun:
		return ShotgunAimTime;
	case EWeaponType::Pistol:
		return PistolAimTime;
	case EWeaponType::Rifle:
		return RifleAimTime;
	case EWeaponType::Knife:
		return KnifeAimTime;
	}

	return 0.2f;
}

float AEnemyShooterCharacter::GetCurrentCooldownTime() const
{
	switch (EnemyWeaponType)
	{
	case EWeaponType::Shotgun:
		return ShotgunCooldownTime;
	case EWeaponType::Pistol:
		return PistolCooldownTime;
	case EWeaponType::Rifle:
		return RifleCooldownTime;
	case EWeaponType::Knife:
		return KnifeCooldownTime;
	}

	return 0.5f;
}
