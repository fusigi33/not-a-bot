#include "Settings/MusicSubsystem.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMusicSubsystem, Log, All);

void UMusicSubsystem::PlayMusic(
	USoundBase* Music,
	float FadeInSeconds,
	float FadeOutSeconds,
	float TransitionDelaySeconds,
	float VolumeMultiplier,
	float StartTime,
	bool bPersistAcrossLevelTransition,
	bool bLooping)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PendingMusicTimerHandle);
	}
	PendingMusic = nullptr;

	if (!Music)
	{
		UE_LOG(LogMusicSubsystem, Warning, TEXT("PlayMusic called with no Music. Stopping active music."));
		StopMusic(FadeOutSeconds);
		return;
	}

	if (ActiveMusicComponent && CurrentMusic == Music)
	{
		UE_LOG(LogMusicSubsystem, Verbose, TEXT("PlayMusic ignored because '%s' is already active."), *Music->GetName());
		bCurrentMusicLoops = bLooping;
		if (!ActiveMusicComponent->IsPlaying())
		{
			const float ResumeTime = CurrentMusicPlaybackTime > 0.0f ? CurrentMusicPlaybackTime : StartTime;
			ActiveMusicComponent->FadeIn(FMath::Max(0.0f, FadeInSeconds), VolumeMultiplier, ResumeTime);
		}
		return;
	}

	FadeOutActiveMusic(FadeOutSeconds);
	CurrentMusic = nullptr;

	const float ClampedTransitionDelaySeconds = FMath::Max(0.0f, TransitionDelaySeconds);
	if (ClampedTransitionDelaySeconds > 0.0f)
	{
		PendingMusic = Music;
		PendingFadeInSeconds = FadeInSeconds;
		PendingVolumeMultiplier = VolumeMultiplier;
		PendingStartTime = StartTime;
		bPendingPersistAcrossLevelTransition = bPersistAcrossLevelTransition;
		bPendingLooping = bLooping;

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				PendingMusicTimerHandle,
				this,
				&UMusicSubsystem::StartPendingMusic,
				ClampedTransitionDelaySeconds,
				false);
			return;
		}
	}

	StartMusicNow(Music, FadeInSeconds, VolumeMultiplier, StartTime, bPersistAcrossLevelTransition, bLooping);
}

void UMusicSubsystem::StartMusicNow(
	USoundBase* Music,
	float FadeInSeconds,
	float VolumeMultiplier,
	float StartTime,
	bool bPersistAcrossLevelTransition,
	bool bLooping)
{
	if (!Music)
	{
		return;
	}

	CurrentMusicPlaybackTime = FMath::Max(0.0f, StartTime);
	CurrentMusicPlaybackPercent = 0.0f;

	UAudioComponent* NewMusicComponent = UGameplayStatics::CreateSound2D(
		GetGameInstance(),
		Music,
		VolumeMultiplier,
		1.0f,
		StartTime,
		nullptr,
		bPersistAcrossLevelTransition,
		false);

	if (!NewMusicComponent)
	{
		UE_LOG(LogMusicSubsystem, Warning, TEXT("Failed to spawn music component for '%s'."), *Music->GetName());
		CurrentMusic = nullptr;
		return;
	}

	ActiveMusicComponent = NewMusicComponent;
	CurrentMusic = Music;
	bCurrentMusicLoops = bLooping;
	NewMusicComponent->OnAudioFinished.AddDynamic(this, &UMusicSubsystem::HandleActiveMusicFinished);
	NewMusicComponent->OnAudioPlaybackPercent.AddDynamic(this, &UMusicSubsystem::HandleActiveMusicPlaybackPercent);
	UE_LOG(LogMusicSubsystem, Log, TEXT("Playing music '%s'."), *Music->GetName());

	if (FadeInSeconds > 0.0f)
	{
		NewMusicComponent->FadeIn(FadeInSeconds, VolumeMultiplier, StartTime);
	}
	else
	{
		NewMusicComponent->Play(StartTime);
	}
}

void UMusicSubsystem::StartPendingMusic()
{
	USoundBase* MusicToStart = PendingMusic;
	PendingMusic = nullptr;

	StartMusicNow(
		MusicToStart,
		PendingFadeInSeconds,
		PendingVolumeMultiplier,
		PendingStartTime,
		bPendingPersistAcrossLevelTransition,
		bPendingLooping);
}

void UMusicSubsystem::HandleActiveMusicFinished()
{
	if (!ActiveMusicComponent || !CurrentMusic || !bCurrentMusicLoops)
	{
		return;
	}

	if (CurrentMusicPlaybackPercent < 0.98f)
	{
		return;
	}

	CurrentMusicPlaybackTime = 0.0f;
	CurrentMusicPlaybackPercent = 0.0f;
	ActiveMusicComponent->Play(0.0f);
}

void UMusicSubsystem::HandleActiveMusicPlaybackPercent(const USoundWave* PlayingSoundWave, const float PlaybackPercent)
{
	CurrentMusicPlaybackPercent = FMath::Clamp(PlaybackPercent, 0.0f, 1.0f);

	if (PlayingSoundWave)
	{
		CurrentMusicPlaybackTime = FMath::Max(0.0f, PlayingSoundWave->GetDuration() * CurrentMusicPlaybackPercent);
	}
}

void UMusicSubsystem::StopMusic(float FadeOutSeconds)
{
	UE_LOG(LogMusicSubsystem, Verbose, TEXT("StopMusic called."));
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PendingMusicTimerHandle);
	}
	PendingMusic = nullptr;
	FadeOutActiveMusic(FadeOutSeconds);
	CurrentMusic = nullptr;
}

bool UMusicSubsystem::IsMusicPlaying() const
{
	return ActiveMusicComponent && ActiveMusicComponent->IsPlaying();
}

void UMusicSubsystem::FadeOutActiveMusic(float FadeOutSeconds)
{
	if (!ActiveMusicComponent)
	{
		return;
	}

	UAudioComponent* ComponentToFade = ActiveMusicComponent;
	ComponentToFade->OnAudioFinished.RemoveDynamic(this, &UMusicSubsystem::HandleActiveMusicFinished);
	ComponentToFade->OnAudioPlaybackPercent.RemoveDynamic(this, &UMusicSubsystem::HandleActiveMusicPlaybackPercent);
	ActiveMusicComponent = nullptr;

	const float ClampedFadeOutSeconds = FMath::Max(0.0f, FadeOutSeconds);
	if (ClampedFadeOutSeconds <= 0.0f)
	{
		ComponentToFade->Stop();
		ComponentToFade->DestroyComponent();
		CleanupFadedMusicComponents();
		return;
	}

	FadingMusicComponents.Add(ComponentToFade);
	ComponentToFade->FadeOut(ClampedFadeOutSeconds, 0.0f);

	if (UWorld* World = GetWorld())
	{
		FTimerHandle CleanupTimerHandle;
		World->GetTimerManager().SetTimer(
			CleanupTimerHandle,
			FTimerDelegate::CreateUObject(this, &UMusicSubsystem::CleanupFadedMusicComponents),
			ClampedFadeOutSeconds + 0.1f,
			false);
	}
}

void UMusicSubsystem::CleanupFadedMusicComponents()
{
	for (int32 Index = FadingMusicComponents.Num() - 1; Index >= 0; --Index)
	{
		UAudioComponent* FadingComponent = FadingMusicComponents[Index];
		if (!FadingComponent || !FadingComponent->IsPlaying())
		{
			if (FadingComponent)
			{
				FadingComponent->DestroyComponent();
			}
			FadingMusicComponents.RemoveAtSwap(Index);
		}
	}
}
