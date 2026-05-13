#include "Settings/MusicSubsystem.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
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
	// Looping is now owned by the sound asset itself. The parameter is kept for Blueprint API compatibility.
	(void)bLooping;

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
		if (!ActiveMusicComponent->IsPlaying())
		{
			ActiveMusicComponent->FadeIn(FMath::Max(0.0f, FadeInSeconds), VolumeMultiplier, StartTime);
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

	StartMusicNow(Music, FadeInSeconds, VolumeMultiplier, StartTime, bPersistAcrossLevelTransition);
}

void UMusicSubsystem::StartMusicNow(
	USoundBase* Music,
	float FadeInSeconds,
	float VolumeMultiplier,
	float StartTime,
	bool bPersistAcrossLevelTransition)
{
	if (!Music)
	{
		return;
	}

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
	NewMusicComponent->OnAudioFinished.AddDynamic(this, &UMusicSubsystem::HandleActiveMusicFinished);
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
		bPendingPersistAcrossLevelTransition);
}

void UMusicSubsystem::HandleActiveMusicFinished()
{
	if (!ActiveMusicComponent)
	{
		return;
	}

	ActiveMusicComponent->DestroyComponent();
	ActiveMusicComponent = nullptr;
	CurrentMusic = nullptr;
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
