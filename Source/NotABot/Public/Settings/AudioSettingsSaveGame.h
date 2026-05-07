#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AudioSettingsSaveGame.generated.h"

UCLASS()
class NOTABOT_API UAudioSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category="Audio")
	float MasterVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category="Audio")
	float SFXVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category="Audio")
	float MusicVolume = 1.f;
};
