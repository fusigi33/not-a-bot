#include "FPS/Weapons/Guns/ShotgunComponent.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UShotgunComponent::UShotgunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShotgunComponent::BeginPlay()
{
	Super::BeginPlay();
}

FVector UShotgunComponent::GetRandomSpreadDirection(const FVector& Forward, float InSpreadDegrees) const
{
	const float ConeHalfAngleRad = FMath::DegreesToRadians(InSpreadDegrees);
	return FMath::VRandCone(Forward, ConeHalfAngleRad, ConeHalfAngleRad).GetSafeNormal();
}

void UShotgunComponent::FireShotgun(
	AActor* Shooter,
	const FVector& Start,
	const FVector& AimDirection,
	float ExtraAimErrorDegrees)
{
	if (!GetWorld() || !Shooter)
	{
		return;
	}

	const float FinalSpread = SpreadAngleDegrees + ExtraAimErrorDegrees;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Shooter);
	Params.bReturnPhysicalMaterial = false;

	for (int32 i = 0; i < PelletCount; ++i)
	{
		const FVector ShotDir = GetRandomSpreadDirection(AimDirection.GetSafeNormal(), FinalSpread);
		const FVector End = Start + ShotDir * Range;

		FHitResult Hit;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECC_Visibility,
			Params
		);

		if (bHit)
		{
			UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				DamagePerPellet,
				Shooter->GetInstigatorController(),
				Shooter,
				UDamageType::StaticClass()
			);

			// 디버그용
			DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
		}
	}
}