#pragma once

#include "CoreMinimal.h"
#include "FPS/Weapons/Base/WeaponBaseComponent.h"
#include "KnifeWeaponComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NOTABOT_API UKnifeWeaponComponent : public UWeaponBaseComponent
{
	GENERATED_BODY()

public:
	UKnifeWeaponComponent();

	UFUNCTION(BlueprintCallable, Category="Knife")
	void ApplyKnifeDamage(AActor* User, const FVector& Start, const FVector& AimDirection);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Knife")
	float AttackRadius = 35.f;

	virtual void PerformUse(
		AActor* User,
		const FVector& Start,
		const FVector& AimDirection) override;
};
