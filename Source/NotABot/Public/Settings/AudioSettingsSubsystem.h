#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSettingsSubsystem.generated.h"

class UAudioSettingsSaveGame;
class USoundClass;
class USoundMix;

UCLASS(BlueprintType)
class NOTABOT_API UAudioSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void ConfigureSoundClasses(
		USoundMix* InSoundMix,
		USoundClass* InMasterSoundClass,
		USoundClass* InSFXSoundClass,
		USoundClass* InMusicSoundClass);

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetMasterVolume(float InVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetSFXVolume(float InVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetMusicVolume(float InVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetVolumes(float InMasterVolume, float InSFXVolume, float InMusicVolume, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void ApplyVolumes();

	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SaveSettings();

	UFUNCTION(BlueprintPure, Category="Audio Settings")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category="Audio Settings")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintPure, Category="Audio Settings")
	float GetMusicVolume() const { return MusicVolume; }

private:
	void LoadSettings();
	static float ClampVolume(float InVolume);

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> ActiveSoundMix = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> MasterSoundClass = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> SFXSoundClass = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> MusicSoundClass = nullptr;

	float MasterVolume = 1.f;
	float SFXVolume = 1.f;
	float MusicVolume = 1.f;
};
