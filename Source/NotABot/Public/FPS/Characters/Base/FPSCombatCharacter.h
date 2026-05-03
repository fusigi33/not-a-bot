#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCombatCharacter.generated.h"

class UWidgetComponent;
class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FFPSCombatCharacterDeathSignature,
	AFPSCombatCharacter*, DeadCharacter,
	AController*, KillerController);

UCLASS(Abstract)
class NOTABOT_API AFPSCombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSCombatCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxArmor = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentArmor = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<class AActor> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FName WeaponAttachSocketName = TEXT("hand_rSocket");

	UPROPERTY()
	AActor* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bDead = false;

	// 아머가 먼저 피해를 얼마나 흡수할지 비율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float ArmorAbsorbRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float InvulnerabilityDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float InvulnerableUntilTime = 0.f;

public:
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FFPSCombatCharacterDeathSignature OnCharacterDied;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AActor* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetArmor() const { return CurrentArmor; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsInvulnerable() const;

protected:
	virtual void Die(AController* KillerController);
	virtual void OnDeath();
	void RestoreEquippedWeaponAttachment();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ActivateSelf();

	void ApplyDamageToArmorAndHealth(float IncomingDamage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void K2_OnDamaged(
		float DamageAmount,
		float NewHealth,
		float NewArmor,
		AController* EventInstigator,
		AActor* DamageCauser);
};
