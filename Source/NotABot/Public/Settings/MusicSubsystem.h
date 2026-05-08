#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(BlueprintType)
class NOTABOT_API UMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Music")
	void PlayMusic(
		USoundBase* Music,
		float FadeInSeconds = 0.5f,
		float FadeOutSeconds = 0.5f,
		float VolumeMultiplier = 1.0f,
		float StartTime = 0.0f,
		bool bPersistAcrossLevelTransition = false,
		bool bLooping = true);

	UFUNCTION(BlueprintCallable, Category="Music")
	void StopMusic(float FadeOutSeconds = 0.5f);

	UFUNCTION(BlueprintPure, Category="Music")
	USoundBase* GetCurrentMusic() const { return CurrentMusic; }

	UFUNCTION(BlueprintPure, Category="Music")
	bool IsMusicPlaying() const;

private:
	void FadeOutActiveMusic(float FadeOutSeconds);
	void CleanupFadedMusicComponents();

	UFUNCTION()
	void HandleActiveMusicFinished();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveMusicComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentMusic = nullptr;

	bool bCurrentMusicLoops = true;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> FadingMusicComponents;
};
