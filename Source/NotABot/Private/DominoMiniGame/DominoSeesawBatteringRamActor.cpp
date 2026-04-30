#include "DominoMiniGame/DominoSeesawBatteringRamActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/BodyInstance.h"
#include "TimerManager.h"

ADominoSeesawBatteringRamActor::ADominoSeesawBatteringRamActor()
{
	// 시소 기울기 연출이 필요할 때만 Tick을 켭니다.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 고정 받침대와 회전하는 막대를 분리해 막대 계층만 기울일 수 있게 구성합니다.
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FulcrumMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FulcrumMesh"));
	FulcrumMesh->SetupAttachment(SceneRoot);
	FulcrumMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FulcrumMesh->SetCollisionObjectType(ECC_WorldStatic);
	FulcrumMesh->SetCollisionResponseToAllChannels(ECR_Block);

	BeamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BeamRoot"));
	BeamRoot->SetupAttachment(SceneRoot);

	// 시소에 달린 메시들은 BeamRoot 아래에 배치해 동일한 회전을 공유합니다.
	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamMesh"));
	BeamMesh->SetupAttachment(BeamRoot);
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BeamMesh->SetCollisionObjectType(ECC_WorldDynamic);
	BeamMesh->SetCollisionResponseToAllChannels(ECR_Block);

	LeftBasketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftBasketMesh"));
	LeftBasketMesh->SetupAttachment(BeamRoot);
	LeftBasketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftBasketMesh->SetCollisionObjectType(ECC_WorldDynamic);
	LeftBasketMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	RightBasketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightBasketMesh"));
	RightBasketMesh->SetupAttachment(BeamRoot);
	RightBasketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightBasketMesh->SetCollisionObjectType(ECC_WorldDynamic);
	RightBasketMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	LeftBasketFloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBasketFloorCollision"));
	LeftBasketFloorCollision->SetupAttachment(LeftBasketMesh);
	LeftBasketFloorCollision->SetCollisionProfileName(TEXT("BlockAll"));
	LeftBasketFloorCollision->SetBoxExtent(FVector(45.0f, 45.0f, 6.0f));
	LeftBasketFloorCollision->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));

	LeftBasketFrontCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBasketFrontCollision"));
	LeftBasketFrontCollision->SetupAttachment(LeftBasketMesh);
	LeftBasketFrontCollision->SetCollisionProfileName(TEXT("BlockAll"));
	LeftBasketFrontCollision->SetBoxExtent(FVector(6.0f, 45.0f, 24.0f));
	LeftBasketFrontCollision->SetRelativeLocation(FVector(45.0f, 0.0f, 0.0f));

	LeftBasketBackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBasketBackCollision"));
	LeftBasketBackCollision->SetupAttachment(LeftBasketMesh);
	LeftBasketBackCollision->SetCollisionProfileName(TEXT("BlockAll"));
	LeftBasketBackCollision->SetBoxExtent(FVector(6.0f, 45.0f, 24.0f));
	LeftBasketBackCollision->SetRelativeLocation(FVector(-45.0f, 0.0f, 0.0f));

	LeftBasketInnerCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBasketInnerCollision"));
	LeftBasketInnerCollision->SetupAttachment(LeftBasketMesh);
	LeftBasketInnerCollision->SetCollisionProfileName(TEXT("BlockAll"));
	LeftBasketInnerCollision->SetBoxExtent(FVector(45.0f, 6.0f, 24.0f));
	LeftBasketInnerCollision->SetRelativeLocation(FVector(0.0f, 45.0f, 0.0f));

	LeftBasketOuterCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBasketOuterCollision"));
	LeftBasketOuterCollision->SetupAttachment(LeftBasketMesh);
	LeftBasketOuterCollision->SetCollisionProfileName(TEXT("BlockAll"));
	LeftBasketOuterCollision->SetBoxExtent(FVector(45.0f, 6.0f, 24.0f));
	LeftBasketOuterCollision->SetRelativeLocation(FVector(0.0f, -45.0f, 0.0f));

	RightBasketFloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBasketFloorCollision"));
	RightBasketFloorCollision->SetupAttachment(RightBasketMesh);
	RightBasketFloorCollision->SetCollisionProfileName(TEXT("BlockAll"));
	RightBasketFloorCollision->SetBoxExtent(FVector(45.0f, 45.0f, 6.0f));
	RightBasketFloorCollision->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));

	RightBasketFrontCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBasketFrontCollision"));
	RightBasketFrontCollision->SetupAttachment(RightBasketMesh);
	RightBasketFrontCollision->SetCollisionProfileName(TEXT("BlockAll"));
	RightBasketFrontCollision->SetBoxExtent(FVector(6.0f, 45.0f, 24.0f));
	RightBasketFrontCollision->SetRelativeLocation(FVector(45.0f, 0.0f, 0.0f));

	RightBasketBackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBasketBackCollision"));
	RightBasketBackCollision->SetupAttachment(RightBasketMesh);
	RightBasketBackCollision->SetCollisionProfileName(TEXT("BlockAll"));
	RightBasketBackCollision->SetBoxExtent(FVector(6.0f, 45.0f, 24.0f));
	RightBasketBackCollision->SetRelativeLocation(FVector(-45.0f, 0.0f, 0.0f));

	RightBasketInnerCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBasketInnerCollision"));
	RightBasketInnerCollision->SetupAttachment(RightBasketMesh);
	RightBasketInnerCollision->SetCollisionProfileName(TEXT("BlockAll"));
	RightBasketInnerCollision->SetBoxExtent(FVector(45.0f, 6.0f, 24.0f));
	RightBasketInnerCollision->SetRelativeLocation(FVector(0.0f, -45.0f, 0.0f));

	RightBasketOuterCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBasketOuterCollision"));
	RightBasketOuterCollision->SetupAttachment(RightBasketMesh);
	RightBasketOuterCollision->SetCollisionProfileName(TEXT("BlockAll"));
	RightBasketOuterCollision->SetBoxExtent(FVector(45.0f, 6.0f, 24.0f));
	RightBasketOuterCollision->SetRelativeLocation(FVector(0.0f, 45.0f, 0.0f));

	// 바구니 충돌 박스는 시각 연출용 BeamRoot와 분리해 충돌 판정을 안정적으로 유지합니다.
	LeftBasketBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBasketBlockerBox"));
	LeftBasketBlocker->SetupAttachment(SceneRoot);
	LeftBasketBlocker->SetBoxExtent(FVector(45.0f, 45.0f, 18.0f));
	LeftBasketBlocker->SetRelativeLocation(FVector(0.0f, -180.0f, -10.0f));
	ConfigureBasketTriggerCollision(LeftBasketBlocker);

	RightBasketBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBasketBlockerBox"));
	RightBasketBlocker->SetupAttachment(SceneRoot);
	RightBasketBlocker->SetBoxExtent(FVector(45.0f, 45.0f, 18.0f));
	RightBasketBlocker->SetRelativeLocation(FVector(0.0f, 180.0f, -10.0f));
	ConfigureBasketTriggerCollision(RightBasketBlocker);
}

void ADominoSeesawBatteringRamActor::BeginPlay()
{
	Super::BeginPlay();

	// 리셋 시 되돌아갈 배치와 런타임 충돌 핸들러를 초기화합니다.
	InitialTransform = GetActorTransform();
	LeftBasketBlocker->OnComponentBeginOverlap.AddDynamic(this, &ADominoSeesawBatteringRamActor::HandleLeftBasketOverlap);
	RightBasketBlocker->OnComponentBeginOverlap.AddDynamic(this, &ADominoSeesawBatteringRamActor::HandleRightBasketOverlap);
	ConfigureBasketTriggerCollision(LeftBasketBlocker);
	ConfigureBasketTriggerCollision(RightBasketBlocker);
	ApplyLayout();
	SetBasketBlockersEnabled(false);
}

void ADominoSeesawBatteringRamActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 디테일 패널에서 바구니 간격을 조정하면 메시 배치를 에디터 뷰포트에 즉시 반영합니다.
	ConfigureBasketTriggerCollision(LeftBasketBlocker);
	ConfigureBasketTriggerCollision(RightBasketBlocker);
	ApplyLayout();
}

void ADominoSeesawBatteringRamActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!BeamRoot)
	{
		return;
	}

	const FRotator CurrentRotation = BeamRoot->GetRelativeRotation();
	const float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, TargetBeamRoll, DeltaSeconds, TiltInterpSpeed);
	BeamRoot->SetRelativeRotation(FRotator(0.0f, 0.0f, NewRoll));

	// 원위치 복귀까지 끝났으면 Tick을 꺼서 불필요한 프레임 처리를 줄입니다.
	if (FMath::IsNearlyEqual(NewRoll, TargetBeamRoll, 0.1f) && FMath::IsNearlyZero(TargetBeamRoll, 0.1f))
	{
		SetActorTickEnabled(false);
	}
}

void ADominoSeesawBatteringRamActor::SetDominoSimulationEnabled_Implementation(bool bEnabled)
{
	// 시뮬레이션 활성화 상태와 실제 바구니 충돌 상태를 함께 관리합니다.
	bSimulationEnabled = bEnabled;
	bArmed = true;
	SetBasketBlockersEnabled(bEnabled);
	SetBasketMeshCollisionBoxesEnabled(EDominoSeesawBasketSide::Left, true);
	SetBasketMeshCollisionBoxesEnabled(EDominoSeesawBasketSide::Right, true);
	if (bEnabled)
	{
		LastBasketBlockerEnabledTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		RegisterExistingBasketBalls();
	}

	if (!bEnabled)
	{
		// 비활성화 시 예약된 재장전을 취소하고 시소를 원위치로 돌립니다.
		GetWorldTimerManager().ClearTimer(RearmTimerHandle);
		RestoreLockedBasketBallMovement();
		LeftBasketBallComponent.Reset();
		RightBasketBallComponent.Reset();
		TargetBeamRoll = 0.0f;
		SetActorTickEnabled(true);
	}
}

void ADominoSeesawBatteringRamActor::ResetDominoSimulationObject_Implementation()
{
	// 물리 오브젝트가 밀어낸 위치와 시소 회전을 모두 초기 상태로 되돌립니다.
	SetDominoSimulationEnabled_Implementation(false);
	SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);
	TargetBeamRoll = 0.0f;
	if (BeamRoot)
	{
		BeamRoot->SetRelativeRotation(FRotator::ZeroRotator);
	}
	SetBasketMeshCollisionBoxesEnabled(EDominoSeesawBasketSide::Left, true);
	SetBasketMeshCollisionBoxesEnabled(EDominoSeesawBasketSide::Right, true);
}

bool ADominoSeesawBatteringRamActor::TryLaunchOppositeBasket(EDominoSeesawBasketSide ImpactSide, UPrimitiveComponent* ImpactingBallComponent)
{
	// 비활성 상태이거나 재장전 대기 중이면 같은 충돌로 중복 발사하지 않습니다.
	if (!bSimulationEnabled || !bArmed)
	{
		return false;
	}

	// 충돌한 쪽의 반대편 바구니에서 발사할 물리 컴포넌트를 찾습니다.
	const EDominoSeesawBasketSide LaunchSide = ImpactSide == EDominoSeesawBasketSide::Left
		? EDominoSeesawBasketSide::Right
		: EDominoSeesawBasketSide::Left;

	UPrimitiveComponent* BallToLaunch = GetBasketBallComponent(LaunchSide);
	if (!BallToLaunch || BallToLaunch == ImpactingBallComponent || !BallToLaunch->IsSimulatingPhysics())
	{
		BallToLaunch = FindLaunchableBallInBasket(LaunchSide, ImpactingBallComponent);
	}

	if (!BallToLaunch)
	{
		return false;
	}

	bArmed = false;
	SetBasketMeshCollisionBoxesEnabled(LaunchSide, false);
	SetBasketBallComponent(LaunchSide, nullptr);
	PrepareBasketBallForLaunch(BallToLaunch);

	// 반대편 공에 포물선 초기 속도를 직접 부여해 즉시 발사합니다.
	BallToLaunch->SetPhysicsLinearVelocity(BuildLaunchVelocity(LaunchSide), false, NAME_None);
	BallToLaunch->WakeAllRigidBodies();

	// 충돌 방향에 맞춰 막대 기울기 목표를 정하고 연출 Tick을 시작합니다.
	TargetBeamRoll = ImpactSide == EDominoSeesawBasketSide::Left ? -MaxTiltDegrees : MaxTiltDegrees;
	SetActorTickEnabled(true);

	OnBallLaunched.Broadcast(ImpactSide, LaunchSide, ImpactingBallComponent, BallToLaunch);

	// 짧은 재장전 시간을 두어 충돌 이벤트 연속 발생을 하나의 작동으로 묶습니다.
	GetWorldTimerManager().SetTimer(RearmTimerHandle, this, &ADominoSeesawBatteringRamActor::Rearm, RearmDelay, false);
	return true;
}

void ADominoSeesawBatteringRamActor::HandleLeftBasketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	HandleBasketOverlap(EDominoSeesawBasketSide::Left, OtherComp);
}

void ADominoSeesawBatteringRamActor::HandleRightBasketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	HandleBasketOverlap(EDominoSeesawBasketSide::Right, OtherComp);
}

void ADominoSeesawBatteringRamActor::HandleBasketOverlap(EDominoSeesawBasketSide ImpactSide, UPrimitiveComponent* OtherComp)
{
	// 물리 시뮬레이션 대상만 시소를 작동시킬 수 있습니다.
	if (!OtherComp || !OtherComp->IsSimulatingPhysics())
	{
		return;
	}

	const float IncomingSpeed = OtherComp->GetPhysicsLinearVelocity().Size();

	StabilizeBasketBall(ImpactSide, OtherComp);
	SetBasketBallComponent(ImpactSide, OtherComp);

	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastBasketBlockerEnabledTime < BasketBlockerActivationGraceTime)
	{
		return;
	}

	if (IncomingSpeed < MinimumBasketImpactSpeed)
	{
		return;
	}

	TryLaunchOppositeBasket(ImpactSide, OtherComp);
}

UPrimitiveComponent* ADominoSeesawBatteringRamActor::FindLaunchableBallInBasket(EDominoSeesawBasketSide BasketSide, UPrimitiveComponent* ComponentToIgnore) const
{
	// 발사 대상은 지정한 바구니 충돌 박스 영역 안에서 별도 쿼리로 찾습니다.
	const UBoxComponent* BasketBlocker = BasketSide == EDominoSeesawBasketSide::Left ? LeftBasketBlocker : RightBasketBlocker;
	const UWorld* World = GetWorld();
	if (!BasketBlocker || !World)
	{
		return nullptr;
	}

	TArray<FOverlapResult> BasketSearchResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DominoSeesawBasketSearch), false, this);
	if (ComponentToIgnore)
	{
		QueryParams.AddIgnoredComponent(ComponentToIgnore);
	}

	const FVector SearchExtent = BasketBlocker->GetScaledBoxExtent() + FVector(BasketSearchPadding);
	World->OverlapMultiByObjectType(
		BasketSearchResults,
		BasketBlocker->GetComponentLocation(),
		BasketBlocker->GetComponentQuat(),
		ObjectQueryParams,
		FCollisionShape::MakeBox(SearchExtent),
		QueryParams);

	for (const FOverlapResult& SearchResult : BasketSearchResults)
	{
		UPrimitiveComponent* Component = SearchResult.GetComponent();
		// 방금 들어온 공은 제외하고, 물리 시뮬레이션 중인 첫 컴포넌트를 발사 대상으로 사용합니다.
		if (Component && Component != ComponentToIgnore && Component->IsSimulatingPhysics())
		{
			return Component;
		}
	}

	return nullptr;
}

UPrimitiveComponent* ADominoSeesawBatteringRamActor::GetBasketBallComponent(EDominoSeesawBasketSide BasketSide) const
{
	return BasketSide == EDominoSeesawBasketSide::Left
		? LeftBasketBallComponent.Get()
		: RightBasketBallComponent.Get();
}

void ADominoSeesawBatteringRamActor::SetBasketBallComponent(EDominoSeesawBasketSide BasketSide, UPrimitiveComponent* BallComponent)
{
	if (BallComponent)
	{
		if (BasketSide != EDominoSeesawBasketSide::Left && LeftBasketBallComponent.Get() == BallComponent)
		{
			LeftBasketBallComponent.Reset();
		}
		else if (BasketSide != EDominoSeesawBasketSide::Right && RightBasketBallComponent.Get() == BallComponent)
		{
			RightBasketBallComponent.Reset();
		}
	}

	if (BasketSide == EDominoSeesawBasketSide::Left)
	{
		LeftBasketBallComponent = BallComponent;
	}
	else
	{
		RightBasketBallComponent = BallComponent;
	}
}

void ADominoSeesawBatteringRamActor::RegisterExistingBasketBalls()
{
	RegisterExistingBasketBall(EDominoSeesawBasketSide::Left);
	RegisterExistingBasketBall(EDominoSeesawBasketSide::Right);
}

void ADominoSeesawBatteringRamActor::RegisterExistingBasketBall(EDominoSeesawBasketSide BasketSide)
{
	UPrimitiveComponent* ExistingBall = FindLaunchableBallInBasket(BasketSide, nullptr);
	if (!ExistingBall)
	{
		return;
	}

	StabilizeBasketBall(BasketSide, ExistingBall);
	SetBasketBallComponent(BasketSide, ExistingBall);
}

FVector ADominoSeesawBatteringRamActor::BuildLaunchVelocity(EDominoSeesawBasketSide LaunchSide) const
{
	// 설정 각도와 속도를 수평/수직 성분으로 나누어 포물선 초기 속도를 만듭니다.
	const float AngleRadians = FMath::DegreesToRadians(FMath::Clamp(LaunchAngleDegrees, 0.0f, 89.0f));
	const float HorizontalSpeed = FMath::Cos(AngleRadians) * LaunchSpeed;
	const float VerticalSpeed = FMath::Sin(AngleRadians) * LaunchSpeed;

	return GetLaunchHorizontalDirection(LaunchSide) * HorizontalSpeed + FVector::UpVector * VerticalSpeed;
}

void ADominoSeesawBatteringRamActor::ApplyLayout()
{
	// 바구니 메시와 충돌 트리거 위치를 현재 오프셋 값에 맞춰 배치합니다.
	if (LeftBasketMesh)
	{
		LeftBasketMesh->SetRelativeLocation(FVector(0.0f, -BasketOffset, 0.0f));
		LeftBasketMesh->UpdateComponentToWorld();
	}

	if (RightBasketMesh)
	{
		RightBasketMesh->SetRelativeLocation(FVector(0.0f, BasketOffset, 0.0f));
		RightBasketMesh->UpdateComponentToWorld();
	}

	if (LeftBasketBlocker)
	{
		const FVector CurrentLocation = LeftBasketBlocker->GetRelativeLocation();
		LeftBasketBlocker->SetRelativeLocation(FVector(CurrentLocation.X, -BasketOffset, CurrentLocation.Z));
		LeftBasketBlocker->UpdateComponentToWorld();
	}

	if (RightBasketBlocker)
	{
		const FVector CurrentLocation = RightBasketBlocker->GetRelativeLocation();
		RightBasketBlocker->SetRelativeLocation(FVector(CurrentLocation.X, BasketOffset, CurrentLocation.Z));
		RightBasketBlocker->UpdateComponentToWorld();
	}
}

void ADominoSeesawBatteringRamActor::SetBasketMeshCollisionBoxesEnabled(EDominoSeesawBasketSide BasketSide, bool bEnabled) const
{
	UBoxComponent* BasketCollisionBoxes[5] = {};
	if (BasketSide == EDominoSeesawBasketSide::Left)
	{
		BasketCollisionBoxes[0] = LeftBasketFloorCollision;
		BasketCollisionBoxes[1] = LeftBasketFrontCollision;
		BasketCollisionBoxes[2] = LeftBasketBackCollision;
		BasketCollisionBoxes[3] = LeftBasketInnerCollision;
		BasketCollisionBoxes[4] = LeftBasketOuterCollision;
	}
	else
	{
		BasketCollisionBoxes[0] = RightBasketFloorCollision;
		BasketCollisionBoxes[1] = RightBasketFrontCollision;
		BasketCollisionBoxes[2] = RightBasketBackCollision;
		BasketCollisionBoxes[3] = RightBasketInnerCollision;
		BasketCollisionBoxes[4] = RightBasketOuterCollision;
	}

	for (UBoxComponent* BasketCollisionBox : BasketCollisionBoxes)
	{
		if (BasketCollisionBox)
		{
			BasketCollisionBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			BasketCollisionBox->SetCollisionResponseToAllChannels(bEnabled ? ECR_Block : ECR_Ignore);
		}
	}
}

void ADominoSeesawBatteringRamActor::SetBasketBlockersEnabled(bool bEnabled) const
{
	// 시뮬레이션 중에는 바구니 트리거가 물리 공의 진입을 감지하고, 그 외에는 완전히 비활성화합니다.
	if (LeftBasketBlocker)
	{
		ConfigureBasketTriggerCollision(LeftBasketBlocker);
		LeftBasketBlocker->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (RightBasketBlocker)
	{
		ConfigureBasketTriggerCollision(RightBasketBlocker);
		RightBasketBlocker->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void ADominoSeesawBatteringRamActor::ConfigureBasketTriggerCollision(UBoxComponent* BasketTrigger) const
{
	if (!BasketTrigger)
	{
		return;
	}

	BasketTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	BasketTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	BasketTrigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	BasketTrigger->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	BasketTrigger->SetGenerateOverlapEvents(true);
	BasketTrigger->SetNotifyRigidBodyCollision(false);
}

void ADominoSeesawBatteringRamActor::Rearm()
{
	// 재장전 후 다음 충돌을 받을 수 있게 하고 시소를 원위치로 보간합니다.
	bArmed = true;
	TargetBeamRoll = 0.0f;
	SetActorTickEnabled(true);
}

void ADominoSeesawBatteringRamActor::LockBasketBallMovement(UPrimitiveComponent* BallComponent)
{
	if (!BallComponent || !BallComponent->IsSimulatingPhysics())
	{
		return;
	}

	for (const FLockedBasketBallMovementState& LockedState : LockedBasketBallMovementStates)
	{
		if (LockedState.Component.Get() == BallComponent)
		{
			return;
		}
	}

	FBodyInstance& BodyInstance = BallComponent->BodyInstance;

	FLockedBasketBallMovementState LockedState;
	LockedState.Component = BallComponent;
	LockedState.OriginalDOFMode = static_cast<uint8>(BodyInstance.DOFMode.GetValue());
	LockedState.bOriginalLockTranslation = BodyInstance.bLockTranslation;
	LockedState.bOriginalLockRotation = BodyInstance.bLockRotation;
	LockedState.bOriginalLockXTranslation = BodyInstance.bLockXTranslation;
	LockedState.bOriginalLockYTranslation = BodyInstance.bLockYTranslation;
	LockedState.bOriginalLockZTranslation = BodyInstance.bLockZTranslation;
	LockedState.bOriginalLockXRotation = BodyInstance.bLockXRotation;
	LockedState.bOriginalLockYRotation = BodyInstance.bLockYRotation;
	LockedState.bOriginalLockZRotation = BodyInstance.bLockZRotation;
	LockedBasketBallMovementStates.Add(LockedState);

	BodyInstance.bLockTranslation = false;
	BodyInstance.bLockXTranslation = true;
	BodyInstance.bLockYTranslation = true;
	BodyInstance.bLockZTranslation = true;
	BodyInstance.SetDOFLock(EDOFMode::SixDOF);
}

void ADominoSeesawBatteringRamActor::StabilizeBasketBall(EDominoSeesawBasketSide BasketSide, UPrimitiveComponent* BallComponent)
{
	if (!BallComponent || !BallComponent->IsSimulatingPhysics())
	{
		return;
	}

	BallComponent->SetWorldLocation(GetBasketHoldLocation(BasketSide), false, nullptr, ETeleportType::TeleportPhysics);
	BallComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
	BallComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
	LockBasketBallMovement(BallComponent);
	BallComponent->PutAllRigidBodiesToSleep();
}

FVector ADominoSeesawBatteringRamActor::GetBasketHoldLocation(EDominoSeesawBasketSide BasketSide) const
{
	const UBoxComponent* BasketBlocker = BasketSide == EDominoSeesawBasketSide::Left ? LeftBasketBlocker : RightBasketBlocker;
	if (!BasketBlocker)
	{
		return GetActorLocation();
	}

	return BasketBlocker->GetComponentLocation() + BasketBlocker->GetUpVector() * BasketBlocker->GetScaledBoxExtent().Z;
}

FVector ADominoSeesawBatteringRamActor::GetLaunchHorizontalDirection(EDominoSeesawBasketSide LaunchSide) const
{
	const UBoxComponent* LaunchBasketBlocker = LaunchSide == EDominoSeesawBasketSide::Left ? LeftBasketBlocker : RightBasketBlocker;
	const UBoxComponent* TargetBasketBlocker = LaunchSide == EDominoSeesawBasketSide::Left ? RightBasketBlocker : LeftBasketBlocker;

	if (LaunchBasketBlocker && TargetBasketBlocker)
	{
		const FVector BasketToBasketDirection = TargetBasketBlocker->GetComponentLocation() - LaunchBasketBlocker->GetComponentLocation();
		const FVector HorizontalDirection = FVector::VectorPlaneProject(BasketToBasketDirection, FVector::UpVector).GetSafeNormal();
		if (!HorizontalDirection.IsNearlyZero())
		{
			return HorizontalDirection;
		}
	}

	const FVector FallbackDirection = LaunchSide == EDominoSeesawBasketSide::Left ? GetActorRightVector() : -GetActorRightVector();
	const FVector HorizontalFallbackDirection = FVector::VectorPlaneProject(FallbackDirection, FVector::UpVector).GetSafeNormal();
	return HorizontalFallbackDirection.IsNearlyZero() ? GetActorForwardVector().GetSafeNormal() : HorizontalFallbackDirection;
}

void ADominoSeesawBatteringRamActor::RestoreLockedBasketBallMovement(UPrimitiveComponent* BallComponent)
{
	if (!BallComponent)
	{
		return;
	}

	for (int32 Index = 0; Index < LockedBasketBallMovementStates.Num(); ++Index)
	{
		const FLockedBasketBallMovementState& LockedState = LockedBasketBallMovementStates[Index];
		if (LockedState.Component.Get() != BallComponent)
		{
			continue;
		}

		FBodyInstance& BodyInstance = BallComponent->BodyInstance;
		BodyInstance.bLockTranslation = LockedState.bOriginalLockTranslation;
		BodyInstance.bLockRotation = LockedState.bOriginalLockRotation;
		BodyInstance.bLockXTranslation = LockedState.bOriginalLockXTranslation;
		BodyInstance.bLockYTranslation = LockedState.bOriginalLockYTranslation;
		BodyInstance.bLockZTranslation = LockedState.bOriginalLockZTranslation;
		BodyInstance.bLockXRotation = LockedState.bOriginalLockXRotation;
		BodyInstance.bLockYRotation = LockedState.bOriginalLockYRotation;
		BodyInstance.bLockZRotation = LockedState.bOriginalLockZRotation;
		BodyInstance.SetDOFLock(static_cast<EDOFMode::Type>(LockedState.OriginalDOFMode));

		LockedBasketBallMovementStates.RemoveAtSwap(Index);
		return;
	}
}

void ADominoSeesawBatteringRamActor::PrepareBasketBallForLaunch(UPrimitiveComponent* BallComponent)
{
	if (!BallComponent)
	{
		return;
	}

	RestoreLockedBasketBallMovement(BallComponent);

	FBodyInstance& BodyInstance = BallComponent->BodyInstance;
	BodyInstance.bLockTranslation = false;
	BodyInstance.bLockXTranslation = false;
	BodyInstance.bLockYTranslation = false;
	BodyInstance.bLockZTranslation = false;
	BodyInstance.SetDOFLock(EDOFMode::None);

	BallComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
	BallComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false, NAME_None);
	BallComponent->WakeAllRigidBodies();
}

void ADominoSeesawBatteringRamActor::RestoreLockedBasketBallMovement()
{
	for (const FLockedBasketBallMovementState& LockedState : LockedBasketBallMovementStates)
	{
		UPrimitiveComponent* Component = LockedState.Component.Get();
		if (!Component)
		{
			continue;
		}

		FBodyInstance& BodyInstance = Component->BodyInstance;
		BodyInstance.bLockTranslation = LockedState.bOriginalLockTranslation;
		BodyInstance.bLockRotation = LockedState.bOriginalLockRotation;
		BodyInstance.bLockXTranslation = LockedState.bOriginalLockXTranslation;
		BodyInstance.bLockYTranslation = LockedState.bOriginalLockYTranslation;
		BodyInstance.bLockZTranslation = LockedState.bOriginalLockZTranslation;
		BodyInstance.bLockXRotation = LockedState.bOriginalLockXRotation;
		BodyInstance.bLockYRotation = LockedState.bOriginalLockYRotation;
		BodyInstance.bLockZRotation = LockedState.bOriginalLockZRotation;
		BodyInstance.SetDOFLock(static_cast<EDOFMode::Type>(LockedState.OriginalDOFMode));
	}

	LockedBasketBallMovementStates.Reset();
}
