#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellCaptureRig.generated.h"

class USceneCaptureComponent2D;
class USceneComponent;
class UTextureRenderTarget2D;

UCLASS()
class NOTABOT_API AShellCaptureRig : public AActor
{
	GENERATED_BODY()

public:
	AShellCaptureRig();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category="Shell Game")
	USceneCaptureComponent2D* GetSceneCapture() const { return SceneCapture; }

	UFUNCTION(BlueprintPure, Category="Shell Game")
	UTextureRenderTarget2D* GetRenderTarget() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;
};
