#pragma once

#include "CoreMinimal.h"
#include "FPS/Weapons/Base/WeaponBaseComponent.h"
#include "ShotgunComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NOTABOT_API UShotgunComponent : public UWeaponBaseComponent
{
	GENERATED_BODY()

public:
	UShotgunComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	int32 PelletCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float DamagePerPellet = 13.f;

	// 플레이어 샷건 산탄 퍼짐 각도(도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float SpreadAngleDegrees = 6.f;

	// AI가 쏠 때 사용하는 추가 오차
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float AIAimErrorDegrees = 4.f;

public:
	bool FireShotgun(
		AActor* Shooter,
		const FVector& Start,
		const FVector& AimDirection,
		float ExtraAimErrorDegrees = 0.f);

protected:
	FVector GetRandomSpreadDirection(const FVector& Forward, float InSpreadDegrees) const;
	virtual void PerformUse(
		AActor* User,
		const FVector& Start,
		const FVector& AimDirection) override;

	float PendingExtraAimErrorDegrees = 0.f;
};
