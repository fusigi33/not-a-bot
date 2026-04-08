#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Shotgun,
	Pistol,
	Rifle,
	Knife
};