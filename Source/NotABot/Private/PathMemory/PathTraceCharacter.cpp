#include "PathMemory/PathTraceCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

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
		UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] RegisterMappingContext skipped: DefaultMappingContext is null. Character=%s"), *GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] RegisterMappingContext skipped: EnhancedInput subsystem is null. Character=%s Controller=%s"),
			*GetName(),
			Controller ? *Controller->GetName() : TEXT("None"));
		return;
	}

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(DefaultMappingContext, 0);

	UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] Registered mapping context. Character=%s Context=%s"),
		*GetName(),
		*DefaultMappingContext->GetName());
}

void APathTraceCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PollRawMovementKeys();

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
		UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] SetupPlayerInputComponent. Character=%s InputComponent=%s MoveAction=%s LookAction=%s CursorAction=%s"),
			*GetName(),
			PlayerInputComponent ? *PlayerInputComponent->GetName() : TEXT("None"),
			MoveAction ? *MoveAction->GetName() : TEXT("None"),
			LookAction ? *LookAction->GetName() : TEXT("None"),
			CursorAction ? *CursorAction->GetName() : TEXT("None"));

		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Started, this, &APathTraceCharacter::LogMoveActionStarted);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Ongoing, this, &APathTraceCharacter::LogMoveActionOngoing);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APathTraceCharacter::Move);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APathTraceCharacter::LogMoveActionCompleted);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &APathTraceCharacter::LogMoveActionCanceled);
			UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] Bound MoveAction=%s"), *MoveAction->GetName());
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
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] SetupPlayerInputComponent skipped: not EnhancedInputComponent. Character=%s InputComponent=%s"),
			*GetName(),
			PlayerInputComponent ? *PlayerInputComponent->GetName() : TEXT("None"));
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

void APathTraceCharacter::PollRawMovementKeys()
{
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	const bool bRawWKeyDown = PC->IsInputKeyDown(EKeys::W);
	const bool bRawAKeyDown = PC->IsInputKeyDown(EKeys::A);
	const bool bRawSKeyDown = PC->IsInputKeyDown(EKeys::S);
	const bool bRawDKeyDown = PC->IsInputKeyDown(EKeys::D);

	if (bRawWKeyDown != bWasRawWKeyDown || bRawAKeyDown != bWasRawAKeyDown || bRawSKeyDown != bWasRawSKeyDown || bRawDKeyDown != bWasRawDKeyDown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] Raw key state changed. W=%s A=%s S=%s D=%s CanMove=%s Controller=%s"),
			bRawWKeyDown ? TEXT("down") : TEXT("up"),
			bRawAKeyDown ? TEXT("down") : TEXT("up"),
			bRawSKeyDown ? TEXT("down") : TEXT("up"),
			bRawDKeyDown ? TEXT("down") : TEXT("up"),
			bCanPlayerMove ? TEXT("true") : TEXT("false"),
			*PC->GetName());

		bWasRawWKeyDown = bRawWKeyDown;
		bWasRawAKeyDown = bRawAKeyDown;
		bWasRawSKeyDown = bRawSKeyDown;
		bWasRawDKeyDown = bRawDKeyDown;
	}
}

void APathTraceCharacter::LogMoveActionStarted(const FInputActionValue& Value)
{
	LogMoveActionEvent(TEXT("Started"), Value);
}

void APathTraceCharacter::LogMoveActionOngoing(const FInputActionValue& Value)
{
	LogMoveActionEvent(TEXT("Ongoing"), Value);
}

void APathTraceCharacter::LogMoveActionCompleted(const FInputActionValue& Value)
{
	LogMoveActionEvent(TEXT("Completed"), Value);
}

void APathTraceCharacter::LogMoveActionCanceled(const FInputActionValue& Value)
{
	LogMoveActionEvent(TEXT("Canceled"), Value);
}

void APathTraceCharacter::LogMoveActionEvent(const TCHAR* EventName, const FInputActionValue& Value) const
{
	const FVector2D MoveValue = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] MoveAction %s X=%.3f Y=%.3f CanMove=%s Action=%s"),
		EventName,
		MoveValue.X,
		MoveValue.Y,
		bCanPlayerMove ? TEXT("true") : TEXT("false"),
		MoveAction ? *MoveAction->GetName() : TEXT("None"));
}

void APathTraceCharacter::Move(const FInputActionValue& Value)
{
	if (!bCanPlayerMove)
	{
		const FVector2D BlockedMoveValue = Value.Get<FVector2D>();
		UE_LOG(LogTemp, Verbose, TEXT("[PathTraceCharacter] Move ignored because player movement is disabled. X=%.3f Y=%.3f"),
			BlockedMoveValue.X,
			BlockedMoveValue.Y);
		return;
	}

	const FVector2D MoveValue = Value.Get<FVector2D>();
	if (MoveValue.IsNearlyZero())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PathTraceCharacter] Move ignored because input is nearly zero. X=%.3f Y=%.3f"),
			MoveValue.X,
			MoveValue.Y);
		return;
	}

	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	UE_LOG(LogTemp, Warning, TEXT("[PathTraceCharacter] Move input X=%.3f Y=%.3f ControlYaw=%.2f Forward=%s Right=%s Location=%s Velocity=%s MovementMode=%d ConstrainToPlane=%s"),
		MoveValue.X,
		MoveValue.Y,
		YawRot.Yaw,
		*ForwardDir.ToCompactString(),
		*RightDir.ToCompactString(),
		*GetActorLocation().ToCompactString(),
		MovementComponent ? *MovementComponent->Velocity.ToCompactString() : TEXT("None"),
		MovementComponent ? static_cast<int32>(MovementComponent->MovementMode) : -1,
		MovementComponent && MovementComponent->bConstrainToPlane ? TEXT("true") : TEXT("false"));

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
