#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponBaseComponent.generated.h"

class UParticleSystem;
class USoundBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NOTABOT_API UWeaponBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponBaseComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float HeadDamageMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float TorsoDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float ArmDamageMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float LegDamageMultiplier = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float HandFootDamageMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float Range = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float FireInterval = 0.2f;

	// 퍼짐 정도(도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float SpreadDegrees = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Effects")
	UParticleSystem* MuzzleFlashFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Effects")
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Effects")
	FName MuzzleSocketName = TEXT("muzzle");

	float LastFireTime = -100.f;

public:
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual bool TryUse(
		AActor* User,
		const FVector& Start,
		const FVector& AimDirection);

protected:
	virtual void PerformUse(
		AActor* User,
		const FVector& Start,
		const FVector& AimDirection);

	FVector ApplySpread(const FVector& Direction) const;

	FHitResult ResolveSkeletalMeshHit(const FHitResult& Hit, const FVector& Start, const FVector& End) const;

	float GetDamageForHit(float BaseDamage, const FHitResult& Hit) const;

	bool CanUse() const;

	void PlayFireEffects(AActor* User, const FVector& FallbackLocation) const;
};
