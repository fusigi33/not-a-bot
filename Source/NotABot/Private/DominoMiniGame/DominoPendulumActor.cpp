#include "DominoMiniGame/DominoPendulumActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ADominoPendulumActor::ADominoPendulumActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	PivotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PivotRoot"));
	SetRootComponent(PivotRoot);

	PivotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PivotMesh"));
	PivotMesh->SetupAttachment(PivotRoot);
	PivotMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmMesh"));
	ArmMesh->SetupAttachment(PivotRoot);
	ArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BobMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BobMesh"));
	BobMesh->SetupAttachment(PivotRoot);
	BobMesh->SetMobility(EComponentMobility::Movable);
	BobMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BobMesh->SetCollisionObjectType(ECC_WorldDynamic);
	BobMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BobMesh->SetNotifyRigidBodyCollision(true);
	BobMesh->SetSimulatePhysics(false);
}

void ADominoPendulumActor::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
	BobMesh->OnComponentHit.AddDynamic(this, &ADominoPendulumActor::HandleBobHit);
	ResetDominoSimulationObject_Implementation();
}

void ADominoPendulumActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SwingTime = 0.0f;
	SwingDirectionSign = 1.0f;
	UpdatePendulum(0.0f, false);
}

void ADominoPendulumActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bSwinging)
	{
		return;
	}

	UpdatePendulum(SwingTime + DeltaSeconds, true);
}

void ADominoPendulumActor::SetDominoSimulationEnabled_Implementation(bool bEnabled)
{
	bSimulationEnabled = bEnabled;

	if (bEnabled && bAutoStartWhenSimulationEnabled)
	{
		StartSwing();
	}
	else if (!bEnabled)
	{
		StopSwing();
	}
}

void ADominoPendulumActor::ResetDominoSimulationObject_Implementation()
{
	bSimulationEnabled = false;
	StopSwing();
	SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SwingTime = 0.0f;
	UpdatePendulum(0.0f, false);
}

void ADominoPendulumActor::StartSwing()
{
	SwingTime = 0.0f;
	bSwinging = true;
	PreviousBobWorldLocation = BobMesh ? BobMesh->GetComponentLocation() : FVector::ZeroVector;
	SetActorTickEnabled(true);
}

void ADominoPendulumActor::ApplyPendulumForce(float ForceDirectionSign)
{
	if (!bSimulationEnabled)
	{
		return;
	}

	SwingDirectionSign = ForceDirectionSign >= 0.0f ? 1.0f : -1.0f;
	StartSwing();
}

void ADominoPendulumActor::StopSwing()
{
	bSwinging = false;
	SetActorTickEnabled(false);
	CurrentBobVelocity = FVector::ZeroVector;
}

void ADominoPendulumActor::HandleBobHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bSimulationEnabled || !OtherComp || !OtherComp->IsSimulatingPhysics())
	{
		return;
	}

	if (!bSwinging)
	{
		FVector ImpactPoint = Hit.ImpactPoint;
		if (ImpactPoint.IsNearlyZero())
		{
			ImpactPoint = OtherComp->GetComponentLocation();
		}

		const FVector IncomingVelocity = OtherComp->GetPhysicsLinearVelocityAtPoint(ImpactPoint);
		UE_LOG(LogTemp, Log, TEXT("[DominoPendulum] Hit by %s/%s at %s. ImpactVelocity=%s Speed=%.2f MinimumActivationSpeed=%.2f"),
			OtherActor ? *OtherActor->GetName() : TEXT("None"),
			OtherComp ? *OtherComp->GetName() : TEXT("None"),
			*ImpactPoint.ToString(),
			*IncomingVelocity.ToString(),
			IncomingVelocity.Size(),
			MinimumActivationSpeed);

		if (IncomingVelocity.Size() < MinimumActivationSpeed)
		{
			return;
		}

		const FVector SwingAxis = PivotRoot ? PivotRoot->GetForwardVector() : GetActorForwardVector();
		const float RawForceSign = FVector::DotProduct(IncomingVelocity, SwingAxis);
		const float ForceSign = bInvertImpactDirection ? -RawForceSign : RawForceSign;
		ApplyPendulumForce(FMath::IsNearlyZero(ForceSign) ? 1.0f : ForceSign);
		return;
	}

	if (!CurrentBobVelocity.IsNearlyZero())
	{
		const FVector Impulse = CurrentBobVelocity * HitImpulseMultiplier;
		OtherComp->AddImpulseAtLocation(Impulse, Hit.ImpactPoint);
	}
}

void ADominoPendulumActor::UpdatePendulum(float NewSwingTime, bool bSweepBob)
{
	if (!BobMesh)
	{
		return;
	}

	const float ClampedHalfDuration = FMath::Max(0.05f, HalfSwingDuration);
	const float MaxAngle = GetMaxSwingAngleRadians();
	const float Angle = SwingDirectionSign * MaxAngle * FMath::Sin((NewSwingTime / ClampedHalfDuration) * PI);

	// PivotRoot 기준의 로컬 좌표로 위치를 계산합니다.
	const FVector LocalBobLocation(
		FMath::Sin(Angle) * PendulumLength,
		0.0f,
		-FMath::Cos(Angle) * PendulumLength
	);

	const FVector OldWorldLocation = BobMesh->GetComponentLocation();

	FHitResult SweepHit;
	BobMesh->SetRelativeLocation(LocalBobLocation, bSweepBob, &SweepHit, ETeleportType::None);

	const FVector ActualBobWorldLocation = BobMesh->GetComponentLocation();
	const FVector ActualBobLocalLocation = GetBobLocalLocation();
	const float DeltaTime = FMath::Max(NewSwingTime - SwingTime, KINDA_SMALL_NUMBER);
	CurrentBobVelocity = (ActualBobWorldLocation - OldWorldLocation) / DeltaTime;
	PreviousBobWorldLocation = ActualBobWorldLocation;
	SwingTime = NewSwingTime;

	UpdateArmVisual(ActualBobLocalLocation);
}

void ADominoPendulumActor::UpdateArmVisual(const FVector& BobLocalLocation)
{
	if (!ArmMesh || !PivotRoot)
	{
		return;
	}

	// 1. 피벗에서 Bob의 피벗(최하단)까지의 거리 계산
	const float PivotToBobDistance = BobLocalLocation.Length();

	if (PivotToBobDistance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// 2. 뚫고 지나가는 현상 해결 (길이 계산)
	// Bob의 피벗이 최하단에 있으므로, '피벗~Bob바닥' 거리에서 'Bob의 높이'를 빼야 합니다.
	// 디테일 패널에서 ArmBobCenterOffset 값을 Bob의 지름(높이)만큼 설정해 주세요.
	const float ArmDistance = FMath::Max(1.0f, PivotToBobDistance - ArmBobCenterOffset);
	
	const FVector ArmDirection = BobLocalLocation / PivotToBobDistance;
	const FVector ArmVector = ArmDirection * ArmDistance;

	// [수정 포인트 1] 위치 설정
	// Arm의 피벗이 이미 최상단에 있다면, 위치를 옮기지 않고 원점(PivotRoot 위치)에 둡니다.
	ArmMesh->SetRelativeLocation(FVector::ZeroVector);

	// 3. 회전 설정 (기존과 동일)
	const FRotator ArmRotation = bArmMeshUsesLocalXAxis
		? ArmVector.Rotation()
		: FRotationMatrix::MakeFromZ(ArmVector.GetSafeNormal()).Rotator();
	ArmMesh->SetRelativeRotation(ArmRotation);

	// [수정 포인트 2] 스케일 설정
	// 피벗이 끝에 있으므로, 계산된 ArmDistance 전체를 메시 원본 길이로 나누어 스케일을 구합니다.
	const float LengthScale = ArmDistance / GetArmMeshSourceLength();
	const FVector ArmScale = bArmMeshUsesLocalXAxis
		? FVector(LengthScale, ArmMeshScaleX, ArmMeshScaleY)
		: FVector(ArmMeshScaleX, ArmMeshScaleY, LengthScale);
	ArmMesh->SetRelativeScale3D(ArmScale);
}

FVector ADominoPendulumActor::GetBobLocalLocation() const
{
	if (!BobMesh || !PivotRoot)
	{
		return FVector::ZeroVector;
	}

	return PivotRoot->GetComponentTransform().InverseTransformPosition(BobMesh->GetComponentLocation());
}

float ADominoPendulumActor::GetArmMeshSourceLength() const
{
	if (bUseArmMeshBoundsForLength && ArmMesh && ArmMesh->GetStaticMesh())
	{
		const FBoxSphereBounds MeshBounds = ArmMesh->GetStaticMesh()->GetBounds();
		const float BoundsLength = bArmMeshUsesLocalXAxis
			? MeshBounds.BoxExtent.X * 2.0f
			: MeshBounds.BoxExtent.Z * 2.0f;

		if (BoundsLength > KINDA_SMALL_NUMBER)
		{
			return BoundsLength;
		}
	}

	return FMath::Max(1.0f, ArmMeshReferenceLength);
}

float ADominoPendulumActor::GetMaxSwingAngleRadians() const
{
	const float SafeLength = FMath::Max(1.0f, PendulumLength);
	const float ClampedHeight = FMath::Clamp(MaxLiftHeight, 0.0f, SafeLength * 1.95f);
	const float CosAngle = FMath::Clamp(1.0f - (ClampedHeight / SafeLength), -1.0f, 1.0f);
	return FMath::Acos(CosAngle);
}
