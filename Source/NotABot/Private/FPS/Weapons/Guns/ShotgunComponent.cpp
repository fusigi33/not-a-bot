#include "FPS/Weapons/Guns/ShotgunComponent.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UShotgunComponent::UShotgunComponent()
{
	FireInterval = 1.f;
	Range = 3000.f;
}

FVector UShotgunComponent::GetRandomSpreadDirection(const FVector& Forward, float InSpreadDegrees) const
{
	const float ConeHalfAngleRad = FMath::DegreesToRadians(InSpreadDegrees);
	return FMath::VRandCone(Forward, ConeHalfAngleRad, ConeHalfAngleRad).GetSafeNormal();
}

bool UShotgunComponent::FireShotgun(
	AActor* Shooter,
	const FVector& Start,
	const FVector& AimDirection,
	float ExtraAimErrorDegrees)
{
	PendingExtraAimErrorDegrees = ExtraAimErrorDegrees;
	const bool bFired = TryUse(Shooter, Start, AimDirection);
	PendingExtraAimErrorDegrees = 0.f;
	return bFired;
}

void UShotgunComponent::PerformUse(
	AActor* User,
	const FVector& Start,
	const FVector& AimDirection)
{
	if (!GetWorld() || !User)
	{
		return;
	}

	const float FinalSpread = SpreadAngleDegrees + PendingExtraAimErrorDegrees;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(User);
	Params.bReturnPhysicalMaterial = false;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	for (int32 i = 0; i < PelletCount; ++i)
	{
		const FVector ShotDir = GetRandomSpreadDirection(AimDirection.GetSafeNormal(), FinalSpread);
		const FVector End = Start + ShotDir * Range;

		FHitResult Hit;
		const bool bHit = GetWorld()->LineTraceSingleByObjectType(
			Hit,
			Start,
			End,
			ObjectQueryParams,
			Params
		);

		if (bHit)
		{
			const FHitResult DamageHit = ResolveSkeletalMeshHit(Hit, Start, End);

			UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				GetDamageForHit(DamagePerPellet, DamageHit),
				User->GetInstigatorController(),
				User,
				UDamageType::StaticClass()
			);

			// 디버그용
			DrawDebugLine(GetWorld(), Start, DamageHit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
		}
	}
}
