#include "FPS/Weapons/Melee/KnifeWeaponComponent.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UKnifeWeaponComponent::UKnifeWeaponComponent()
{
	Damage = 40.f;
	Range = 180.f;
	FireInterval = 0.6f;
	SpreadDegrees = 0.f;
	AttackRadius = 35.f;
}

void UKnifeWeaponComponent::PerformUse(
	AActor* User,
	const FVector& Start,
	const FVector& AimDirection)
{
	if (!GetWorld() || !User)
	{
		return;
	}

	const FVector End = Start + AimDirection.GetSafeNormal() * Range;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(User);

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (bHit)
	{
		UGameplayStatics::ApplyDamage(
			Hit.GetActor(),
			Damage,
			User->GetInstigatorController(),
			User,
			UDamageType::StaticClass()
		);

		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, AttackRadius, 16, FColor::Red, false, 1.0f);
		DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);
	}
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1.0f, 0, 1.0f);
	}
}