#pragma once

#include "CoreMinimal.h"
#include "FPS/Characters/Base/FPSCombatCharacter.h"
#include "FPSPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPistolWeaponComponent;
class URifleWeaponComponent;
class UKnifeWeaponComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EPlayerWeaponType : uint8
{
	Shotgun,
	Pistol,
	Rifle,
	Knife
};

UCLASS()
class NOTABOT_API AFPSPlayerCharacter : public AFPSCombatCharacter
{
	GENERATED_BODY()

public:
	AFPSPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UPistolWeaponComponent* PistolComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	URifleWeaponComponent* RifleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UKnifeWeaponComponent* KnifeComponent;

	// 기존 샷건 유지 시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	class UShotgunComponent* ShotgunComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	EPlayerWeaponType CurrentWeapon = EPlayerWeaponType::Shotgun;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* FireAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* EquipShotgunAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* EquipPistolAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* EquipRifleAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* EquipKnifeAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fire();
	void StartFire();
	void StopFire();

	void EquipShotgun();
	void EquipPistol();
	void EquipRifle();
	void EquipKnife();
	
	bool bWantsToFire = false;
};