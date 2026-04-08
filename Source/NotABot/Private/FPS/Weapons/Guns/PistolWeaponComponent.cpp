#include "FPS/Weapons/Guns/PistolWeaponComponent.h"

UPistolWeaponComponent::UPistolWeaponComponent()
{
	Damage = 25.f;
	Range = 6000.f;
	FireInterval = 0.35f;
	SpreadDegrees = 0.75f;
}
