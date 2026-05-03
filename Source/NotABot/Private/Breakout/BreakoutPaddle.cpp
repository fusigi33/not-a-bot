#include "Breakout/BreakoutPaddle.h"

#include "Breakout/BreakoutBrick.h"
#include "Breakout/BreakoutGameManager.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"

ABreakoutPaddle::ABreakoutPaddle()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_Pawn);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionBox->SetNotifyRigidBodyCollision(false);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABreakoutPaddle::OnPaddleBeginOverlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionBox);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ABreakoutPaddle::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation();
	FindGameManagerIfMissing();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputMappingContext)
				{
					InputSubsystem->AddMappingContext(InputMappingContext, 0);
				}
			}
		}
	}
}

void ABreakoutPaddle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ApplyMovement(DeltaSeconds);
}

void ABreakoutPaddle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABreakoutPaddle::MoveInput);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABreakoutPaddle::MoveInput);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ABreakoutPaddle::MoveInput);
		}
	}
}

float ABreakoutPaddle::GetNormalizedHitOffset(const FVector& HitLocation) const
{
	const FVector RightAxis = GetMoveAxis();
	const float SignedDistance = FVector::DotProduct(HitLocation - GetActorLocation(), RightAxis);
	return FMath::Clamp(SignedDistance / FMath::Max(EffectiveHalfWidth, 1.0f), -1.0f, 1.0f);
}

FVector ABreakoutPaddle::GetLaunchUpDirection() const
{
	return UpDirection.GetSafeNormal();
}

FVector ABreakoutPaddle::GetMoveAxis() const
{
	return GetActorRightVector().GetSafeNormal();
}

void ABreakoutPaddle::MoveInput(const FInputActionValue& Value)
{
	if (!CanMove())
	{
		CurrentInput = 0.0f;
		return;
	}

	CurrentInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
}

void ABreakoutPaddle::ApplyMovement(float DeltaSeconds)
{
	if (!CanMove())
	{
		CurrentInput = 0.0f;
		SmoothedInput = 0.0f;
		return;
	}

	const float BlendAlpha = (InputSmoothing >= 1.0f) ? 1.0f : FMath::Clamp(InputSmoothing * DeltaSeconds * 10.0f, 0.0f, 1.0f);
	SmoothedInput = FMath::Lerp(SmoothedInput, CurrentInput, BlendAlpha);

	const FVector MoveAxis = GetMoveAxis();
	const float CurrentOffset = FVector::DotProduct(GetActorLocation() - InitialLocation, MoveAxis);
	const float DeltaMove = SmoothedInput * MoveSpeed * DeltaSeconds;
	const float TargetOffset = FMath::Clamp(CurrentOffset + DeltaMove, -MaxTravelDistance, MaxTravelDistance);
	const FVector TargetLocation = InitialLocation + (MoveAxis * TargetOffset);

	SetActorLocation(TargetLocation, true);
}

bool ABreakoutPaddle::CanMove() const
{
	return GameManager && GameManager->IsMiniGameActive();
}

void ABreakoutPaddle::FindGameManagerIfMissing()
{
	if (GameManager)
	{
		return;
	}

	for (TActorIterator<ABreakoutGameManager> It(GetWorld()); It; ++It)
	{
		GameManager = *It;
		return;
	}
}

void ABreakoutPaddle::OnPaddleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (ABreakoutBrick* Brick = Cast<ABreakoutBrick>(OtherActor))
	{
		if (ABreakoutGameManager* BrickOwnerGameManager = Cast<ABreakoutGameManager>(Brick->GetOwner()))
		{
			BrickOwnerGameManager->HandleBrickTouchedPaddle(Brick);
		}
	}
}
