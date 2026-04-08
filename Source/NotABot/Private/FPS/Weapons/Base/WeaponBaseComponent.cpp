#include "FPS/Weapons/Base/WeaponBaseComponent.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UWeaponBaseComponent::UWeaponBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponBaseComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UWeaponBaseComponent::CanUse() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastFireTime) >= FireInterval;
}

bool UWeaponBaseComponent::TryUse(
	AActor* User,
	const FVector& Start,
	const FVector& AimDirection)
{
	if (!User || !CanUse())
	{
		return false;
	}

	PerformUse(User, Start, AimDirection);
	LastFireTime = GetWorld()->GetTimeSeconds();
	return true;
}

FVector UWeaponBaseComponent::ApplySpread(const FVector& Direction) const
{
	if (SpreadDegrees <= 0.f)
	{
		return Direction.GetSafeNormal();
	}

	const float ConeHalfAngleRad = FMath::DegreesToRadians(SpreadDegrees);
	return FMath::VRandCone(Direction.GetSafeNormal(), ConeHalfAngleRad, ConeHalfAngleRad).GetSafeNormal();
}

void UWeaponBaseComponent::PerformUse(
	AActor* User,
	const FVector& Start,
	const FVector& AimDirection)
{
	if (!GetWorld() || !User)
	{
		return;
	}

	const FVector FinalDir = ApplySpread(AimDirection);
	const FVector End = Start + FinalDir * Range;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(User);

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
			Damage,
			User->GetInstigatorController(),
			User,
			UDamageType::StaticClass()
		);

		DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);
	}
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
	}
}