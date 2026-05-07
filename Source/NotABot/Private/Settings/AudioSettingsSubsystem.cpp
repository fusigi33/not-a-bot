#include "Settings/AudioSettingsSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Settings/AudioSettingsSaveGame.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

namespace
{
const FString AudioSettingsSlotName = TEXT("AudioSettings");
constexpr int32 AudioSettingsUserIndex = 0;
}

void UAudioSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSettings();
}

void UAudioSettingsSubsystem::ConfigureSoundClasses(
	USoundMix* InSoundMix,
	USoundClass* InMasterSoundClass,
	USoundClass* InSFXSoundClass,
	USoundClass* InMusicSoundClass)
{
	ActiveSoundMix = InSoundMix;
	MasterSoundClass = InMasterSoundClass;
	SFXSoundClass = InSFXSoundClass;
	MusicSoundClass = InMusicSoundClass;

	ApplyVolumes();
}

void UAudioSettingsSubsystem::SetMasterVolume(float InVolume, bool bSaveImmediately)
{
	MasterVolume = ClampVolume(InVolume);
	ApplyVolumes();

	if (bSaveImmediately)
	{
		SaveSettings();
	}
}

void UAudioSettingsSubsystem::SetSFXVolume(float InVolume, bool bSaveImmediately)
{
	SFXVolume = ClampVolume(InVolume);
	ApplyVolumes();

	if (bSaveImmediately)
	{
		SaveSettings();
	}
}

void UAudioSettingsSubsystem::SetMusicVolume(float InVolume, bool bSaveImmediately)
{
	MusicVolume = ClampVolume(InVolume);
	ApplyVolumes();

	if (bSaveImmediately)
	{
		SaveSettings();
	}
}

void UAudioSettingsSubsystem::SetVolumes(
	float InMasterVolume,
	float InSFXVolume,
	float InMusicVolume,
	bool bSaveImmediately)
{
	MasterVolume = ClampVolume(InMasterVolume);
	SFXVolume = ClampVolume(InSFXVolume);
	MusicVolume = ClampVolume(InMusicVolume);

	ApplyVolumes();

	if (bSaveImmediately)
	{
		SaveSettings();
	}
}

void UAudioSettingsSubsystem::ApplyVolumes()
{
	if (!ActiveSoundMix)
	{
		return;
	}

	UObject* WorldContext = GetGameInstance();
	if (!WorldContext)
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(WorldContext, ActiveSoundMix);

	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			WorldContext,
			ActiveSoundMix,
			MasterSoundClass,
			MasterVolume,
			1.f,
			0.f,
			true);
	}

	if (SFXSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			WorldContext,
			ActiveSoundMix,
			SFXSoundClass,
			SFXVolume,
			1.f,
			0.f,
			true);
	}

	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			WorldContext,
			ActiveSoundMix,
			MusicSoundClass,
			MusicVolume,
			1.f,
			0.f,
			true);
	}
}

void UAudioSettingsSubsystem::SaveSettings()
{
	UAudioSettingsSaveGame* SaveGame = Cast<UAudioSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UAudioSettingsSaveGame::StaticClass()));

	if (!SaveGame)
	{
		return;
	}

	SaveGame->MasterVolume = MasterVolume;
	SaveGame->SFXVolume = SFXVolume;
	SaveGame->MusicVolume = MusicVolume;

	UGameplayStatics::SaveGameToSlot(SaveGame, AudioSettingsSlotName, AudioSettingsUserIndex);
}

void UAudioSettingsSubsystem::LoadSettings()
{
	if (!UGameplayStatics::DoesSaveGameExist(AudioSettingsSlotName, AudioSettingsUserIndex))
	{
		return;
	}

	UAudioSettingsSaveGame* SaveGame = Cast<UAudioSettingsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(AudioSettingsSlotName, AudioSettingsUserIndex));

	if (!SaveGame)
	{
		return;
	}

	MasterVolume = ClampVolume(SaveGame->MasterVolume);
	SFXVolume = ClampVolume(SaveGame->SFXVolume);
	MusicVolume = ClampVolume(SaveGame->MusicVolume);
}

float UAudioSettingsSubsystem::ClampVolume(float InVolume)
{
	return FMath::Clamp(InVolume, 0.f, 1.f);
}
