#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCombatCharacter.generated.h"

class UWidgetComponent;

UCLASS()
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bDead = false;

	// 아머가 먼저 피해를 얼마나 흡수할지 비율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float ArmorAbsorbRatio = 0.7f;

public:
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetArmor() const { return CurrentArmor; }

protected:
	virtual void Die(AController* KillerController);
	virtual void OnDeath();

	void ApplyDamageToArmorAndHealth(float IncomingDamage);
};