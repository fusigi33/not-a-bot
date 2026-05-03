#include "PathMemory/PathTraceCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"

APathTraceCharacter::APathTraceCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 450.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 80.f);
	SpringArm->bDoCollisionTest = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	PlayerCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PlayerCapture"));
	PlayerCapture->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	PlayerCapture->bCaptureEveryFrame = false;
	PlayerCapture->bCaptureOnMovement = false;
	PlayerCapture->FOVAngle = 95.0f;
	PlayerCapture->SetAutoActivate(false);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void APathTraceCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetPlayerCaptureEnabled(false);
	ResetRecordedPath();
	RegisterMappingContext();
}

UEnhancedInputLocalPlayerSubsystem* APathTraceCharacter::GetEnhancedInputSubsystem() const
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	}

	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

void APathTraceCharacter::RegisterMappingContext()
{
	if (!DefaultMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
	{
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void APathTraceCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bCanPlayerMove)
	{
		return;
	}

	const FVector Current = GetActorLocation();

	if (RecordedPath.Num() == 0 || FVector::Dist2D(Current, LastRecordedLocation) >= RecordMinDistance)
	{
		RecordedPath.Add(Current);
		LastRecordedLocation = Current;
	}
}

void APathTraceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APathTraceCharacter::Move);
		}
		
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APathTraceCharacter::Look);
		}

		if (CursorAction)
		{
			EnhancedInput->BindAction(CursorAction, ETriggerEvent::Started, this, &APathTraceCharacter::ShowMouseCursorWhilePlayerTurn);
			EnhancedInput->BindAction(CursorAction, ETriggerEvent::Completed, this, &APathTraceCharacter::HideMouseCursor);
			EnhancedInput->BindAction(CursorAction, ETriggerEvent::Canceled, this, &APathTraceCharacter::HideMouseCursor);
		}
	}
}

void APathTraceCharacter::ShowMouseCursorWhilePlayerTurn()
{
	if (!bCanPlayerMove)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
	}
}

void APathTraceCharacter::HideMouseCursor()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->IsInputKeyDown(EKeys::LeftAlt) || PC->IsInputKeyDown(EKeys::RightAlt))
		{
			return;
		}

		PC->bShowMouseCursor = false;
	}
}

void APathTraceCharacter::ForceHideMouseCursor()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = false;
	}
}

void APathTraceCharacter::Move(const FInputActionValue& Value)
{
	if (!bCanPlayerMove)
	{
		return;
	}

	const FVector2D MoveValue = Value.Get<FVector2D>();
	if (MoveValue.IsNearlyZero())
	{
		return;
	}

	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MoveValue.Y);
	AddMovementInput(RightDir, MoveValue.X);
}

void APathTraceCharacter::Look(const FInputActionValue& Value)
{
	if (!bCanPlayerMove)
	{
		return;
	}

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->IsInputKeyDown(EKeys::LeftAlt) || PC->IsInputKeyDown(EKeys::RightAlt))
		{
			return;
		}
	}

	const FVector2D LookValue = Value.Get<FVector2D>();
	
	if (!LookValue.IsNearlyZero())
	{
		// 마우스 좌우 이동 -> 컨트롤러 Yaw(Z축 회전) 추가
		AddControllerYawInput(LookValue.X);
		
		// 마우스 상하 이동 -> 컨트롤러 Pitch(Y축 회전) 추가 (언리얼은 기본적으로 상하 반전일 수 있어서 -1을 곱하기도 합니다)
		AddControllerPitchInput(LookValue.Y);
	}
}

void APathTraceCharacter::SetCanPlayerMove(bool bEnable)
{
	bCanPlayerMove = bEnable;
	if (!bCanPlayerMove)
	{
		ForceHideMouseCursor();
	}
}

void APathTraceCharacter::SetPlayerCaptureEnabled(bool bEnable)
{
	if (!PlayerCapture)
	{
		return;
	}

	PlayerCapture->bCaptureEveryFrame = bEnable;
	PlayerCapture->bCaptureOnMovement = bEnable;
	PlayerCapture->SetComponentTickEnabled(bEnable);
	PlayerCapture->SetActive(bEnable);
}

void APathTraceCharacter::ResetRecordedPath()
{
	RecordedPath.Reset();
	LastRecordedLocation = GetActorLocation();
	RecordedPath.Add(LastRecordedLocation);
}
