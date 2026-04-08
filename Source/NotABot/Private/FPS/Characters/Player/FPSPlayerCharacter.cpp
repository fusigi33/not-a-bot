#include "FPS/Characters/Player/FPSPlayerCharacter.h"

#include "FPS/Weapons/Guns/ShotgunComponent.h"
#include "FPS/Weapons/Guns/PistolWeaponComponent.h"
#include "FPS/Weapons/Guns/RifleWeaponComponent.h"
#include "FPS/Weapons/Melee/KnifeWeaponComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

AFPSPlayerCharacter::AFPSPlayerCharacter()
{
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	ShotgunComponent = CreateDefaultSubobject<UShotgunComponent>(TEXT("ShotgunComponent"));
	PistolComponent = CreateDefaultSubobject<UPistolWeaponComponent>(TEXT("PistolComponent"));
	RifleComponent = CreateDefaultSubobject<URifleWeaponComponent>(TEXT("RifleComponent"));
	KnifeComponent = CreateDefaultSubobject<UKnifeWeaponComponent>(TEXT("KnifeComponent"));
}

void AFPSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

void AFPSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::Move);
	}

	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::Look);
	}

	if (FireAction)
	{
		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AFPSPlayerCharacter::Fire);
		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AFPSPlayerCharacter::StartFire);
		EIC->BindAction(FireAction, ETriggerEvent::Completed, this, &AFPSPlayerCharacter::StopFire);
		
	}

	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	if (EquipShotgunAction)
	{
		EIC->BindAction(EquipShotgunAction, ETriggerEvent::Started, this, &AFPSPlayerCharacter::EquipShotgun);
	}

	if (EquipPistolAction)
	{
		EIC->BindAction(EquipPistolAction, ETriggerEvent::Started, this, &AFPSPlayerCharacter::EquipPistol);
	}

	if (EquipRifleAction)
	{
		EIC->BindAction(EquipRifleAction, ETriggerEvent::Started, this, &AFPSPlayerCharacter::EquipRifle);
	}

	if (EquipKnifeAction)
	{
		EIC->BindAction(EquipKnifeAction, ETriggerEvent::Started, this, &AFPSPlayerCharacter::EquipKnife);
	}
}

void AFPSPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bWantsToFire || bDead || CurrentWeapon != EPlayerWeaponType::Rifle || !RifleComponent || !FirstPersonCamera)
	{
		return;
	}

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector Dir = FirstPersonCamera->GetForwardVector();
	RifleComponent->TryUse(this, Start, Dir);
}

void AFPSPlayerCharacter::StartFire()
{
	bWantsToFire = true;

	if (CurrentWeapon != EPlayerWeaponType::Rifle)
	{
		Fire();
	}
}

void AFPSPlayerCharacter::StopFire()
{
	bWantsToFire = false;
}

void AFPSPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	AddMovementInput(GetActorForwardVector(), Input.Y);
	AddMovementInput(GetActorRightVector(), Input.X);
}

void AFPSPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AFPSPlayerCharacter::Fire()
{
	if (bDead || !FirstPersonCamera)
	{
		return;
	}

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector Dir = FirstPersonCamera->GetForwardVector();

	switch (CurrentWeapon)
	{
	case EPlayerWeaponType::Shotgun:
		if (ShotgunComponent)
		{
			ShotgunComponent->FireShotgun(this, Start, Dir, 0.f);
		}
		break;

	case EPlayerWeaponType::Pistol:
		if (PistolComponent)
		{
			PistolComponent->TryUse(this, Start, Dir);
		}
		break;

	case EPlayerWeaponType::Rifle:
		if (RifleComponent)
		{
			RifleComponent->TryUse(this, Start, Dir);
		}
		break;

	case EPlayerWeaponType::Knife:
		if (KnifeComponent)
		{
			KnifeComponent->TryUse(this, Start, Dir);
		}
		break;
	}
}

void AFPSPlayerCharacter::EquipShotgun()
{
	CurrentWeapon = EPlayerWeaponType::Shotgun;
}

void AFPSPlayerCharacter::EquipPistol()
{
	CurrentWeapon = EPlayerWeaponType::Pistol;
}

void AFPSPlayerCharacter::EquipRifle()
{
	CurrentWeapon = EPlayerWeaponType::Rifle;
}

void AFPSPlayerCharacter::EquipKnife()
{
	CurrentWeapon = EPlayerWeaponType::Knife;
}