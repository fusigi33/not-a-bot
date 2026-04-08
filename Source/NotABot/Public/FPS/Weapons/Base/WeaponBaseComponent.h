#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponBaseComponent.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float Range = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float FireInterval = 0.2f;

	// 퍼짐 정도(도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float SpreadDegrees = 0.f;

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

	bool CanUse() const;
};