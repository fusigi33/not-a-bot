#pragma once

#include "CoreMinimal.h"
#include "FPS/Weapons/Base/WeaponBaseComponent.h"
#include "RifleWeaponComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NOTABOT_API URifleWeaponComponent : public UWeaponBaseComponent
{
	GENERATED_BODY()

public:
	URifleWeaponComponent();
};