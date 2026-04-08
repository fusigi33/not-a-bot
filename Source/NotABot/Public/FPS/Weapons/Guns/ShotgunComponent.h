#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShotgunComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NOTABOT_API UShotgunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShotgunComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	int32 PelletCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float DamagePerPellet = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float Range = 3000.f;

	// 플레이어 샷건 산탄 퍼짐 각도(도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float SpreadAngleDegrees = 6.f;

	// AI가 쏠 때 사용하는 추가 오차
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shotgun")
	float AIAimErrorDegrees = 4.f;

public:
	void FireShotgun(
		AActor* Shooter,
		const FVector& Start,
		const FVector& AimDirection,
		float ExtraAimErrorDegrees = 0.f);

protected:
	FVector GetRandomSpreadDirection(const FVector& Forward, float InSpreadDegrees) const;
};