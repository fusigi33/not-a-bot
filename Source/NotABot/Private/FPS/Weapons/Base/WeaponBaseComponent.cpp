#include "FPS/Weapons/Base/WeaponBaseComponent.h"

#include "FPS/Characters/Base/FPSCombatCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

namespace
{
bool BoneNameContains(const FName BoneName, const TCHAR* Text)
{
	return BoneName.ToString().Contains(Text, ESearchCase::IgnoreCase);
}
}

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

void UWeaponBaseComponent::PlayFireEffects(AActor* User, const FVector& FallbackLocation) const
{
	UWorld* World = GetWorld();
	if (!World || !User)
	{
		return;
	}

	USceneComponent* AttachComponent = nullptr;
	if (const AFPSCombatCharacter* CombatCharacter = Cast<AFPSCombatCharacter>(User))
	{
		if (AActor* EquippedWeapon = CombatCharacter->GetEquippedWeapon())
		{
			if (USkeletalMeshComponent* WeaponMesh = EquippedWeapon->FindComponentByClass<USkeletalMeshComponent>();
				WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
			{
				AttachComponent = WeaponMesh;
			}
			else
			{
				AttachComponent = EquippedWeapon->GetRootComponent();
			}
		}
	}

	if (AttachComponent)
	{
		if (MuzzleFlashFX)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlashFX, AttachComponent, MuzzleSocketName);
		}

		if (FireSound)
		{
			UGameplayStatics::SpawnSoundAttached(FireSound, AttachComponent, MuzzleSocketName);
		}

		return;
	}

	if (MuzzleFlashFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlashFX, FallbackLocation);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, FireSound, FallbackLocation);
	}
}

bool UWeaponBaseComponent::TryUse(
	AActor* User,
	const FVector& Start,
	const FVector& AimDirection)
{
	if (!TryStartUse(User, Start))
	{
		return false;
	}

	PerformUse(User, Start, AimDirection);
	return true;
}

bool UWeaponBaseComponent::TryStartUse(AActor* User, const FVector& Start)
{
	if (!User || !CanUse())
	{
		return false;
	}

	PlayFireEffects(User, Start);
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

FHitResult UWeaponBaseComponent::ResolveSkeletalMeshHit(
	const FHitResult& Hit,
	const FVector& Start,
	const FVector& End) const
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor || Hit.BoneName != NAME_None)
	{
		return Hit;
	}

	USkeletalMeshComponent* MeshComponent = HitActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComponent)
	{
		return Hit;
	}

	FCollisionQueryParams MeshParams(SCENE_QUERY_STAT(WeaponMeshHit), true);
	MeshParams.bReturnPhysicalMaterial = false;

	FHitResult MeshHit;
	if (MeshComponent->LineTraceComponent(MeshHit, Start, End, MeshParams))
	{
		return MeshHit;
	}

	return Hit;
}

float UWeaponBaseComponent::GetDamageForHit(float BaseDamage, const FHitResult& Hit) const
{
	const FName BoneName = Hit.BoneName;
	if (BoneName == NAME_None)
	{
		return BaseDamage;
	}

	if (BoneNameContains(BoneName, TEXT("head")) || BoneNameContains(BoneName, TEXT("neck")))
	{
		return BaseDamage * HeadDamageMultiplier;
	}

	if (BoneNameContains(BoneName, TEXT("hand")) || BoneNameContains(BoneName, TEXT("foot")) ||
		BoneNameContains(BoneName, TEXT("ball")))
	{
		return BaseDamage * HandFootDamageMultiplier;
	}

	if (BoneNameContains(BoneName, TEXT("upperarm")) || BoneNameContains(BoneName, TEXT("lowerarm")) ||
		BoneNameContains(BoneName, TEXT("clavicle")))
	{
		return BaseDamage * ArmDamageMultiplier;
	}

	if (BoneNameContains(BoneName, TEXT("thigh")) || BoneNameContains(BoneName, TEXT("calf")))
	{
		return BaseDamage * LegDamageMultiplier;
	}

	if (BoneNameContains(BoneName, TEXT("pelvis")) || BoneNameContains(BoneName, TEXT("spine")))
	{
		return BaseDamage * TorsoDamageMultiplier;
	}

	return BaseDamage;
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
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

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
			GetDamageForHit(Damage, DamageHit),
			User->GetInstigatorController(),
			User,
			UDamageType::StaticClass()
		);

		DrawDebugLine(GetWorld(), Start, DamageHit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);
	}
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
	}
}
