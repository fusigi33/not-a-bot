#pragma once

#include "CoreMinimal.h"
#include "FPS/Characters/Base/FPSCombatCharacter.h"
#include "FPS/Weapons/Data/WeaponTypes.h"
#include "EnemyShooterCharacter.generated.h"

class UShotgunComponent;
class UPistolWeaponComponent;
class URifleWeaponComponent;
class UKnifeWeaponComponent;

UENUM(BlueprintType)
enum class EEnemyPoseType : uint8
{
	Standing,
	Crouching,
	CeilingAttached,
	HidingCorner
};

UENUM(BlueprintType)
enum class EEnemyAttackState : uint8
{
	Idle,
	Aiming,
	FiringBurst,
	Cooldown
};

UCLASS()
class NOTABOT_API AEnemyShooterCharacter : public AFPSCombatCharacter
{
	GENERATED_BODY()

public:
	AEnemyShooterCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UShotgunComponent* ShotgunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UPistolWeaponComponent* PistolComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	URifleWeaponComponent* RifleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UKnifeWeaponComponent* KnifeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	EWeaponType EnemyWeaponType = EWeaponType::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	EEnemyPoseType PoseType = EEnemyPoseType::Standing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	bool bCanAttack = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	EEnemyAttackState AttackState = EEnemyAttackState::Idle;

	// ---------- 공통 조정값 ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Aim")
	float AimInaccuracyDegrees = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Aim")
	float LoseTargetGraceTime = 1.0f;

	// ---------- 권총 ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Pistol")
	float PistolAimTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Pistol")
	float PistolCooldownTime = 0.8f;

	// ---------- 라이플 ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Rifle")
	float RifleAimTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Rifle")
	int32 RifleBurstCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Rifle")
	float RifleBurstInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Rifle")
	float RifleCooldownTime = 0.9f;

	// ---------- 샷건 ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Shotgun")
	float ShotgunAimTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Shotgun")
	float ShotgunCooldownTime = 1.4f;

	// ---------- 칼 ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Knife")
	float KnifeAimTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Knife")
	float KnifeCooldownTime = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Knife")
	float KnifeAttackRange = 180.f;

	// 상태용 시간값
	float StateStartTime = 0.f;
	float LastSeenTargetTime = -100.f;
	float LastBurstShotTime = -100.f;

	int32 BurstShotsFired = 0;

	void HandleInitialPose();

	AActor* GetTargetActor() const;
	FVector GetAttackStartLocation() const;
	FVector GetAimDirectionToTarget(AActor* Target, bool bApplySpread = false) const;
	bool IsTargetInKnifeRange(AActor* Target) const;
	bool HasValidTarget() const;

	void UpdateAttackState(float DeltaSeconds);
	void EnterState(EEnemyAttackState NewState);

	void UpdateIdleState(AActor* Target);
	void UpdateAimingState(AActor* Target);
	void UpdateFiringBurstState(AActor* Target);
	void UpdateCooldownState(AActor* Target);

	bool FireCurrentWeapon(AActor* Target);
	float GetCurrentAimTime() const;
	float GetCurrentCooldownTime() const;
};