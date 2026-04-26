#include "BallShooter/BallShooterRotatingObstacle.h"

#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

ABallShooterRotatingObstacle::ABallShooterRotatingObstacle()
{
	PrimaryActorTick.bCanEverTick = true;

	for (int32 BladeIndex = 0; BladeIndex < 8; ++BladeIndex)
	{
		const FName ComponentName(*FString::Printf(TEXT("BladeCollision%d"), BladeIndex));
		UBoxComponent* BladeCollision = CreateDefaultSubobject<UBoxComponent>(ComponentName);
		BladeCollision->SetupAttachment(SceneRoot);
		ConfigureObstacleCollision(BladeCollision);
		BladeCollisions.Add(BladeCollision);
	}

	UpdateBladeCollisionState();
}

void ABallShooterRotatingObstacle::SetRotationSpeed(float InDegreesPerSecond, bool bClockwise)
{
	DegreesPerSecond = FMath::Abs(InDegreesPerSecond) * (bClockwise ? -1.0f : 1.0f);
}

void ABallShooterRotatingObstacle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateBladeCollisionState();
}

void ABallShooterRotatingObstacle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Rotate continuously around the configured local axis.
	const FVector Axis = LocalRotationAxis.GetSafeNormal();
	if (Axis.IsNearlyZero() || FMath::IsNearlyZero(DegreesPerSecond))
	{
		return;
	}
	
	// 각도를 변수에 직접 누적하고 360도를 넘어가면 오버플로우를 방지하도록 나머지 연산을 해줍니다.
	AccumulatedAngle += DegreesPerSecond * DeltaSeconds;
	AccumulatedAngle = FMath::Fmod(AccumulatedAngle, 360.0f);
	
	const FQuat LocalRotation(Axis, FMath::DegreesToRadians(AccumulatedAngle));
	
	// Base 클래스에서 캐싱해둔 StartRotation(초기 트랜스폼)에 로컬 회전값을 곱하여 절대적 회전 상태를 만듭니다.
	// TeleportPhysics를 사용하여 회전 도중 물리 엔진의 Depenetration(밀어내기) 힘이 개입해 회전을 방해하는 현상도 완벽히 차단합니다.
	SetActorRotation(StartRotation.Quaternion() * LocalRotation, ETeleportType::TeleportPhysics);

	// const FQuat DeltaRotation(Axis, FMath::DegreesToRadians(DegreesPerSecond * DeltaSeconds));
	
	// Do not sweep while rotating; directional collision hits can otherwise clamp one rotation direction more than the other.
	// AddActorLocalRotation(DeltaRotation, false);
}

void ABallShooterRotatingObstacle::DrawObstacleDebug() const
{
	if (!bDrawDebug || !GetWorld())
	{
		return;
	}

	for (int32 BladeIndex = 0; BladeIndex < BladeCollisions.Num(); ++BladeIndex)
	{
		const UBoxComponent* BladeCollision = BladeCollisions[BladeIndex];
		if (!BladeCollision || !BladeCollision->IsCollisionEnabled() || BladeIndex >= BladeCount)
		{
			continue;
		}

		DrawDebugBox(GetWorld(), BladeCollision->GetComponentLocation(), BladeCollision->GetScaledBoxExtent(), BladeCollision->GetComponentQuat(), DebugColor, true, -1.0f, 0, 2.0f);
	}
}

void ABallShooterRotatingObstacle::UpdateBladeCollisionState()
{
	const int32 ActiveBladeCount = FMath::Clamp(BladeCount, 1, BladeCollisions.Num());

	for (int32 BladeIndex = 0; BladeIndex < BladeCollisions.Num(); ++BladeIndex)
	{
		UBoxComponent* BladeCollision = BladeCollisions[BladeIndex];
		if (!BladeCollision)
		{
			continue;
		}

		const bool bIsActive = BladeIndex < ActiveBladeCount;
		BladeCollision->SetHiddenInGame(!bIsActive);
		BladeCollision->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

		if (!bIsActive)
		{
			continue;
		}
	}
}
