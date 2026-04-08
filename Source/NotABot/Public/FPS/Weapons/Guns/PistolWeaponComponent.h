#pragma once

#include "CoreMinimal.h"
#include "FPS/Weapons/Base/WeaponBaseComponent.h"
#include "PistolWeaponComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NOTABOT_API UPistolWeaponComponent : public UWeaponBaseComponent
{
	GENERATED_BODY()

public:
	UPistolWeaponComponent();
};